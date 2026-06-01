import csv
import random
from pathlib import Path

output_dir = Path("data")
output_dir.mkdir(exist_ok=True)

file_path = output_dir / "obd_data.csv"

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

    for _ in range(5000):
        label = random.choices(
            ["SLOW", "NORMAL", "AGGRESSIVE"],
            weights=[0.25, 0.55, 0.20]
        )[0]

        if label == "SLOW":
            speed = random.uniform(0, 50)
            rpm = random.uniform(800, 2200)
            throttle = random.uniform(5, 35)
        elif label == "NORMAL":
            speed = random.uniform(40, 100)
            rpm = random.uniform(1500, 3500)
            throttle = random.uniform(20, 60)
        else:
            speed = random.uniform(90, 150)
            rpm = random.uniform(3500, 6200)
            throttle = random.uniform(55, 100)

        coolant_temp = random.uniform(80, 105)
        fuel_level = random.uniform(5, 100)
        intake_air_temp = random.uniform(15, 40)

        writer.writerow([
            round(speed, 2),
            round(rpm, 2),
            round(throttle, 2),
            round(coolant_temp, 2),
            round(fuel_level, 2),
            round(intake_air_temp, 2),
            label
        ])

print(f"Generated {file_path} with 5000 rows")