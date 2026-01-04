import pygame
import numpy as np
import random
import math

# --- CONFIGURATION ---
WIDTH, HEIGHT = 800, 600
NUM_DRONES = 10
DRONE_SIZE = 15          
SEPARATION_RADIUS = 70   
MAX_SPEED = 3.0          # REDUCED: Was 5.0. Slower, more stable flight.
MAX_FORCE = 0.1          # REDUCED: Was 0.15. Smoother turning.
VIBRATION_INTENSITY = 0.1  # Reduced jitter slightly for stability

# Colors
BACKGROUND = (10, 10, 15)
GREY = (80, 80, 80)      # Disarmed (Darker grey)
RED = (255, 50, 50)      # Armed
WHITE = (255, 255, 255)  
TARGET_COLOR = (255, 255, 0)

class Drone:
    def __init__(self, x, y, id):
        self.pos = np.array([x, y], dtype='float64')
        self.vel = np.array([random.uniform(-1, 1), random.uniform(-1, 1)], dtype='float64')
        self.acc = np.array([0, 0], dtype='float64')
        self.id = id
        self.state = "DISARMED" 
        self.angle = 0

    def apply_force(self, force):
        self.acc += force

    def update(self):
        self.vel += self.acc
        
        # --- SPEED LIMITING BASED ON STATE ---
        curr_speed = np.linalg.norm(self.vel)
        
        limit = MAX_SPEED
        if self.state == "DISARMED":
            limit = 0.5 # Very slow drift when Disarmed
            
        if curr_speed > limit:
            self.vel = (self.vel / curr_speed) * limit
        
        self.pos += self.vel
        self.acc *= 0 
        
        # Calculate Angle
        if curr_speed > 0.1:
            self.angle = math.degrees(math.atan2(-self.vel[1], self.vel[0])) - 90
        
        # Screen Boundary Bounce
        if self.pos[0] < 20 or self.pos[0] > WIDTH - 20: 
            self.vel[0] *= -1
            self.pos[0] = np.clip(self.pos[0], 20, WIDTH-20)
        if self.pos[1] < 20 or self.pos[1] > HEIGHT - 20: 
            self.vel[1] *= -1
            self.pos[1] = np.clip(self.pos[1], 20, HEIGHT-20)

    def behaviors(self, drones, target_pos):
        if self.state == "DISARMED":
            # Just drift slightly (GPS Error simulation)
            wander = np.array([random.uniform(-0.5, 0.5), random.uniform(-0.5, 0.5)])
            self.apply_force(wander * 0.05)
            
        elif self.state == "ARMED":
            seek = self.seek(target_pos)
            separate = self.separate(drones)
            cohesion = self.cohesion(drones)
            
            self.apply_force(seek * 1.0)
            self.apply_force(separate * 3.5)  # Increased separation slightly
            self.apply_force(cohesion * 0.05) 

    def seek(self, target):
        desired = target - self.pos
        dist = np.linalg.norm(desired)
        if dist < 100:
            m = (dist / 100) * MAX_SPEED
            desired = (desired / dist) * m
        else:
            desired = (desired / dist) * MAX_SPEED
        steer = desired - self.vel
        if np.linalg.norm(steer) > MAX_FORCE:
            steer = (steer / np.linalg.norm(steer)) * MAX_FORCE
        return steer

    def separate(self, drones):
        steer = np.array([0, 0], dtype='float64')
        count = 0
        for other in drones:
            d = np.linalg.norm(self.pos - other.pos)
            if 0 < d < SEPARATION_RADIUS:
                diff = self.pos - other.pos
                diff = diff / d 
                steer += diff
                count += 1
        if count > 0:
            steer = steer / count
            if np.linalg.norm(steer) > 0:
                steer = (steer / np.linalg.norm(steer)) * MAX_SPEED
            steer -= self.vel
            if np.linalg.norm(steer) > MAX_FORCE:
                steer = (steer / np.linalg.norm(steer)) * MAX_FORCE
        return steer

    def cohesion(self, drones):
        sum_pos = np.array([0, 0], dtype='float64')
        count = 0
        for other in drones:
            d = np.linalg.norm(self.pos - other.pos)
            if 0 < d < 200: 
                sum_pos += other.pos
                count += 1
        if count > 0:
            sum_pos = sum_pos / count
            return self.seek(sum_pos)
        return np.array([0, 0], dtype='float64')

    def draw(self, screen):
        jitter = random.uniform(-VIBRATION_INTENSITY, VIBRATION_INTENSITY)
        draw_angle = self.angle + jitter
        color = RED if self.state == "ARMED" else GREY
        
        points = [(0, -DRONE_SIZE), (-DRONE_SIZE/1.5, DRONE_SIZE), (DRONE_SIZE/1.5, DRONE_SIZE)]
        rad_angle = math.radians(draw_angle)
        cos_a = math.cos(rad_angle)
        sin_a = math.sin(rad_angle)
        
        rotated_points = []
        for p in points:
            x_new = p[0] * cos_a - p[1] * sin_a + self.pos[0]
            y_new = p[0] * sin_a + p[1] * cos_a + self.pos[1]
            rotated_points.append((x_new, y_new))
            
        pygame.draw.polygon(screen, color, rotated_points)

def main():
    pygame.init()
    screen = pygame.display.set_mode((WIDTH, HEIGHT))
    pygame.display.set_caption("Project S.C.A.N. - Swarm Physics Final")
    clock = pygame.time.Clock()
    font = pygame.font.SysFont("monospace", 18)

    drones = [Drone(random.randint(100, 700), random.randint(100, 500), i+1) for i in range(NUM_DRONES)]
    target_pos = np.array([WIDTH/2, HEIGHT/2], dtype='float64')
    status_msg = "DISARMED: Systems Idle."

    running = True
    while running:
        screen.fill(BACKGROUND)
        for event in pygame.event.get():
            if event.type == pygame.QUIT: running = False
            elif event.type == pygame.KEYDOWN:
                if event.key == pygame.K_c:
                    status_msg = "CALIBRATED: Swarm Armed. Waiting for Target."
                    for d in drones:
                        d.state = "ARMED"
                        d.vel *= 0 
                        d.angle = 0 
                elif event.key == pygame.K_x:
                    status_msg = "ABORT: Scattering."
                    for d in drones:
                        d.state = "DISARMED"
                        d.pos = np.array([random.uniform(50, WIDTH-50), random.uniform(50, HEIGHT-50)], dtype='float64')
            elif event.type == pygame.MOUSEBUTTONDOWN:
                if drones[0].state == "ARMED":
                    target_pos = np.array(pygame.mouse.get_pos(), dtype='float64')
                    status_msg = f"MOVING TO: {int(target_pos[0]), int(target_pos[1])}"

        if drones[0].state == "ARMED":
            pygame.draw.line(screen, TARGET_COLOR, (target_pos[0]-10, target_pos[1]), (target_pos[0]+10, target_pos[1]), 2)
            pygame.draw.line(screen, TARGET_COLOR, (target_pos[0], target_pos[1]-10), (target_pos[0], target_pos[1]+10), 2)

        for drone in drones:
            drone.behaviors(drones, target_pos)
            drone.update()
            drone.draw(screen)

        text = font.render(status_msg, True, WHITE)
        screen.blit(text, (10, 10))
        control_text = font.render("[C] Arm | [Click] Move | [X] Disarm", True, (150, 150, 150))
        screen.blit(control_text, (10, HEIGHT - 30))

        pygame.display.flip()
        clock.tick(60)
    pygame.quit()

if __name__ == "__main__":
    main()