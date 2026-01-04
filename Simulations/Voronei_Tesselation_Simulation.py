import pygame
import math
import random

# --- CONFIGURATION ---
WIDTH, HEIGHT = 800, 600
DRONE_SIZE = 12
SENSOR_RADIUS = 25 # if use 7 then some mines would not be deected so dynamically set the width of travel 
SPEED = 1               # REDUCED: Was 3. Now 1 for smooth, precise scanning.
VIBRATION = 1           # Reduced vibration slightly to match slower speed

# LAYOUT CONSTANTS
MARGIN = 20
ROW_HEIGHT = 40  #Change them in accordance of sensor radius and speed

# Colors
BLACK = (10, 10, 10)
RED = (255, 50, 50)
GREEN = (50, 255, 50)
GREY = (60, 60, 60)
WHITE = (255, 255, 255)
CYAN = (0, 255, 255)
YELLOW = (255, 255, 0)
ORANGE = (255, 165, 0)
PURPLE = (200, 50, 255)
DARK_OVERLAY = (0, 0, 0, 180) # Semi-transparent black for end screen

# --- CLASS DEFINITIONS ---

class MineField:
    def __init__(self, counts):
        self.mines = []
        self.total_mines = sum(counts)
        
        # Quadrant Definitions (x_min, y_min, x_max, y_max)
        areas = [
            (0, 0, WIDTH//2, HEIGHT//2),       # Area 1, Top-Left  (left, top, right, bottom) bounds
            (WIDTH//2, 0, WIDTH, HEIGHT//2),   # Area 2, Top-Right
            (0, HEIGHT//2, WIDTH//2, HEIGHT),  # Area 3, Bottom-Left
            (WIDTH//2, HEIGHT//2, WIDTH, HEIGHT) # Area 4, Bottom-Right
        ]

        for i, count in enumerate(counts):
            area = areas[i]
            for _ in range(count):
                # Random pos within quadrant with padding
                mx = random.randint(area[0] + 25, area[2] - 25)  #Here 25 is padding from edges
                my = random.randint(area[1] + 25, area[3] - 25)
                self.mines.append({
                    'pos': [mx, my],
                    'detected': False,
                    'detected_by': None
                })

class Drone:
    def __init__(self, drone_id, boundary_rect, color):
        self.id = drone_id
        self.bounds = boundary_rect
        self.color = color
        self.reset_system()

    def reset_system(self):
        start_x = self.bounds[0] + MARGIN
        start_y = self.bounds[1] + MARGIN
        
        self.pos = [start_x, start_y]
        self.state = "IDLE"
        self.move_state = "MOVE_X"
        self.direction = 1
        self.next_y = start_y
        self.path = []
        self.angle = 90
        self.score = 0
        self.path.append(tuple(self.pos))

    def arm(self):
        if self.state != "FINISHED":
            self.state = "SCANNING"

    def stop(self):
        if self.state == "SCANNING":
            self.state = "PAUSED"

    def update(self, minefield):
        if self.state != "SCANNING":
            return

        min_x, min_y, max_x, max_y = self.bounds

        # --- MOVEMENT LOGIC ---
        if self.move_state == "MOVE_X":
            self.pos[0] += SPEED * self.direction
            self.angle = 90 if self.direction == 1 else 270

            hit_right = (self.direction == 1 and self.pos[0] >= max_x - MARGIN)
            hit_left = (self.direction == -1 and self.pos[0] <= min_x + MARGIN)

            if hit_right or hit_left:
                self.pos[0] = (max_x - MARGIN) if hit_right else (min_x + MARGIN)
                self.move_state = "MOVE_Y"
                self.next_y = self.pos[1] + ROW_HEIGHT

        elif self.move_state == "MOVE_Y":
            self.pos[1] += SPEED
            self.angle = 180
            
            if self.pos[1] >= self.next_y:
                self.pos[1] = self.next_y
                self.move_state = "MOVE_X"
                self.direction *= -1
                
                # Check for finish (Bottom of own quadrant)
                if self.pos[1] >= max_y - MARGIN:
                    self.state = "FINISHED"

        # --- PATH RECORDING ---
        if abs(self.path[-1][0] - self.pos[0]) > 5 or abs(self.path[-1][1] - self.pos[1]) > 5:
            self.path.append(tuple(self.pos))

        # --- SENSOR LOGIC ---
        for mine in minefield.mines:
            if not mine['detected']:
                dist = math.hypot(self.pos[0] - mine['pos'][0], self.pos[1] - mine['pos'][1])
                if dist < SENSOR_RADIUS:
                    # Verify mine is in this drone's bounds
                    mx, my = mine['pos']
                    if (min_x <= mx <= max_x) and (min_y <= my <= max_y):
                        mine['detected'] = True
                        mine['detected_by'] = self.id
                        self.score += 1

    def draw(self, screen):
        jitter = random.uniform(-VIBRATION, VIBRATION) if self.state == "SCANNING" else 0
        final_angle = self.angle + jitter

        points = [(0, -DRONE_SIZE), (-DRONE_SIZE/1.5, DRONE_SIZE), (DRONE_SIZE/1.5, DRONE_SIZE)]
        rad_angle = math.radians(final_angle)
        cos_a = math.cos(rad_angle)
        sin_a = math.sin(rad_angle)
        
        rotated_points = []
        for p in points:
            x_new = p[0] * cos_a - p[1] * sin_a + self.pos[0]
            y_new = p[0] * sin_a + p[1] * cos_a + self.pos[1]
            rotated_points.append((x_new, y_new))
            
        if self.state == "SCANNING":
            pygame.draw.circle(screen, self.color, (int(self.pos[0]), int(self.pos[1])), SENSOR_RADIUS, 1)
        
        pygame.draw.polygon(screen, self.color, rotated_points)

# --- HELPER FUNCTIONS ---

def get_user_config():
    print("\n=== DRONE FLEET COMMAND ===")
    try:
        q1 = int(input("Mines for Area 1 (Top-Left): "))
        q2 = int(input("Mines for Area 2 (Top-Right): "))
        q3 = int(input("Mines for Area 3 (Bot-Left): "))
        q4 = int(input("Mines for Area 4 (Bot-Right): "))
        print("Configuration Accepted. Launching GUI...")
        return [q1, q2, q3, q4]
    except ValueError:
        print("Invalid Input. Auto-setting to 9 mines each.")
        return [9, 9, 9, 9]

def draw_hud(screen, drones, field, font, font_large):
    # Draw Quadrant Dividers
    pygame.draw.line(screen, GREY, (WIDTH//2, 0), (WIDTH//2, HEIGHT), 2)
    pygame.draw.line(screen, GREY, (0, HEIGHT//2), (WIDTH, HEIGHT//2), 2)

    # Check if Mission Complete (All drones finished)
    all_finished = all(d.state == "FINISHED" for d in drones)

    if all_finished:
        # Draw Results Box in Center
        total_found = sum(d.score for d in drones)
        
        # Semi-transparent box
        s = pygame.Surface((400, 200))
        s.set_alpha(200)
        s.fill(BLACK)
        screen.blit(s, (WIDTH//2 - 200, HEIGHT//2 - 100))
        pygame.draw.rect(screen, GREEN, (WIDTH//2 - 200, HEIGHT//2 - 100, 400, 200), 2)

        # Text
        msg1 = font_large.render("MISSION ACCOMPLISHED", True, GREEN)
        msg2 = font_large.render(f"TOTAL MINES: {total_found}", True, WHITE)
        msg3 = font.render("Press [K] to Reset", True, GREY)

        screen.blit(msg1, (WIDTH//2 - msg1.get_width()//2, HEIGHT//2 - 50))
        screen.blit(msg2, (WIDTH//2 - msg2.get_width()//2, HEIGHT//2))
        screen.blit(msg3, (WIDTH//2 - msg3.get_width()//2, HEIGHT//2 + 50))

    else:
        # Standard Info Panel
        pygame.draw.rect(screen, BLACK, (0, HEIGHT-30, WIDTH, 30))
        pygame.draw.line(screen, WHITE, (0, HEIGHT-30), (WIDTH, HEIGHT-30), 1)
        
        total_found = sum(d.score for d in drones)
        status_text = f"DETECTED: {total_found} | START: [A][B][C][D] | STOP: [W][X][Y][Z]"
        text = font.render(status_text, True, WHITE)
        screen.blit(text, (20, HEIGHT - 25))

    # Individual Drone Stats
    stats = [
        (10, 10, drones[0]), 
        (WIDTH - 150, 10, drones[1]),
        (10, HEIGHT//2 + 10, drones[2]),
        (WIDTH - 150, HEIGHT//2 + 10, drones[3])
    ]

    for x, y, d in stats:
        label = f"D{d.id}: {d.state}"
        count = f"Found: {d.score}"
        col = d.color if d.state != "FINISHED" else GREY
        
        s1 = font.render(label, True, col)
        s2 = font.render(count, True, WHITE)
        screen.blit(s1, (x, y))
        screen.blit(s2, (x, y + 20))

def main():
    counts = get_user_config()
    
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Project S.C.A.N. - Multi-Agent System")
    
    font = pygame.font.SysFont("monospace", 14, bold=True)
    font_large = pygame.font.SysFont("monospace", 30, bold=True)
    
    field = MineField(counts)

    # Initialize 4 Drones
    drones = [
        Drone(1, (0, 0, WIDTH//2, HEIGHT//2), CYAN),
        Drone(2, (WIDTH//2, 0, WIDTH, HEIGHT//2), GREEN),
        Drone(3, (0, HEIGHT//2, WIDTH//2, HEIGHT), ORANGE),
        Drone(4, (WIDTH//2, HEIGHT//2, WIDTH, HEIGHT), PURPLE)
    ]

    clock = pygame.time.Clock()
    running = True

    while running:
        screen.fill(BLACK)
        
        for event in pygame.event.get():
            if event.type == pygame.QUIT: running = False
            
            if event.type == pygame.KEYDOWN:
                # Global Reset
                if event.key == pygame.K_k:
                    field = MineField(counts)
                    for d in drones: d.reset_system()
                
                # Start Controls
                elif event.key == pygame.K_a: drones[0].arm()
                elif event.key == pygame.K_b: drones[1].arm()
                elif event.key == pygame.K_c: drones[2].arm()
                elif event.key == pygame.K_d: drones[3].arm()

                # Stop Controls
                elif event.key == pygame.K_w: drones[0].stop()
                elif event.key == pygame.K_x: drones[1].stop()
                elif event.key == pygame.K_y: drones[2].stop()
                elif event.key == pygame.K_z: drones[3].stop()

        # Update Logic
        for d in drones:
            d.update(field)

        # Drawing
        for d in drones:
            if len(d.path) > 1:
                pygame.draw.lines(screen, d.color, False, d.path, 1)

        for mine in field.mines:
            if mine['detected']:
                pygame.draw.circle(screen, RED, mine['pos'], 8)
                pygame.draw.circle(screen, WHITE, mine['pos'], 10, 1)
            else:
                pygame.draw.circle(screen, (30, 30, 30), mine['pos'], 5)

        for d in drones:
            d.draw(screen)
            
        draw_hud(screen, drones, field, font, font_large)

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()

if __name__ == "__main__":
    main()