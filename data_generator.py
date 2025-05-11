#!/usr/bin/env python3
import sqlite3
import json
import random
from datetime import datetime, timedelta

# Database path
DB_PATH = '/var/lib/grafana/lora_data.db'

# Sample data generation
def generate_sample_deliveries():
    """Generates dummy data for 5 sample deliveries spread over the last 2 weeks"""
    conn = sqlite3.connect(DB_PATH)
    cursor = conn.cursor()

    current_date = datetime.now()

    deliveries = []

    # Define delivery parameters
    for i in range(5):
        # Randomize the date (between 1 and 14 days ago)
        days_ago = random.randint(2, 14)
        delivery_date = current_date - timedelta(days=days_ago)

        # Randomize the time of day
        hour = random.randint(8, 18)  # Deliveries between 8 AM and 6 PM
        minute = random.randint(0, 59)
        second = random.randint(0, 59)

        timestamp = delivery_date.replace(hour=hour, minute=minute, second=second).isoformat()

        # Create random but realistic values
        weight = random.choice([350, 420, 780, 1250, 2300, 3100])  # Common parcel weights in grams
        rssi = random.randint(-100, -60)  # Typical RSSI values
        snr = round(random.uniform(5.0, 15.0), 2)  # Typical SNR values
        packets = random.randint(1, 5)
        counter = i + 1

        # Generate a realistic looking message
        message = f"PARCEL DELIVERY #{counter}"

        deliveries.append({
            'timestamp': timestamp,
            'message': message,
            'rssi': rssi,
            'snr': snr,
            'packets': packets,
            'weight': weight,
            'counter': counter
        })

    # Insert the generated data into the database
    for delivery in deliveries:
        cursor.execute('''
        INSERT INTO lora_messages (timestamp, message, rssi, snr, packets, weight, counter)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (
            delivery['timestamp'],
            delivery['message'],
            delivery['rssi'],
            delivery['snr'],
            delivery['packets'],
            delivery['weight'],
            delivery['counter']
        ))
        print(f"Added delivery: {delivery['timestamp']} - Weight: {delivery['weight']}g")

    # Commit the changes and close the connection
    conn.commit()
    conn.close()
    print("Sample data generation complete!")

if __name__ == "__main__":
    generate_sample_deliveries()
