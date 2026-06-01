import csv
import random
from pathlib import Path

output_dir = Path("data")
output_dir.mkdir(exist_ok=True)

file_path = output_dir / "obd_data.csv"

speed = 0.0
rpm = 900.0
throttle = 10.0
coolant_temp = 82.0
fuel_level = 100.0
intake_air_temp = 24.0

def clamp(value, min_value, max_value):
    return max(min_value, min(value, max_value))

with file_path.open("w", newline="", encoding="utf-8") as file:
    writer = csv.writer(file)

    writer.writerow([
        "speed_kmh",
        "engine_rpm",
        "throttle_pos",
        "coolant_temp",
        "fuel_level",
        "intake_air_temp",
        "label"
    ])

    for i in range(5000):
        phase = (i // 400) % 5

        if phase == 0:      # city slow
            target_speed = random.uniform(20, 50)
        elif phase == 1:    # normal road
            target_speed = random.uniform(55, 85)
        elif phase == 2:    # acceleration
            target_speed = random.uniform(90, 125)
        elif phase == 3:    # calm driving
            target_speed = random.uniform(60, 90)
        else:               # traffic
            target_speed = random.uniform(5, 40)

        speed += (target_speed - speed) * 0.04 + random.uniform(-1.0, 1.0)
        speed = clamp(speed, 0, 150)

        throttle = clamp(10 + (target_speed - speed) * 1.5 + random.uniform(0, 20), 5, 100)
        rpm = clamp(800 + speed * 35 + throttle * 18 + random.uniform(-120, 120), 800, 6200)

        coolant_temp += random.uniform(-0.15, 0.25)
        coolant_temp = clamp(coolant_temp, 80, 105)

        fuel_level -= random.uniform(0.002, 0.015)
        fuel_level = clamp(fuel_level, 5, 100)

        intake_air_temp += random.uniform(-0.08, 0.08)
        intake_air_temp = clamp(intake_air_temp, 15, 40)

        if speed < 50 and rpm < 2500 and throttle < 45:
            label = "SLOW"
        elif speed > 90 and rpm > 3300 and throttle > 50:
            label = "AGGRESSIVE"
        else:
            label = "NORMAL"

        writer.writerow([
            round(speed, 2),
            round(rpm, 2),
            round(throttle, 2),
            round(coolant_temp, 2),
            round(fuel_level, 2),
            round(intake_air_temp, 2),
            label
        ])

print(f"Generated smooth trip dataset: {file_path}")