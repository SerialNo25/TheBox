#!/usr/bin/env python3
import sqlite3
import argparse
import random
from datetime import datetime

# Database path
DB_PATH = '/var/lib/grafana/lora_data.db'

def insert_single_sample(date_str, time_str, weight, rssi, snr):
    """
    Insert a single sample into the database with specified parameters.

    Args:
        date_str (str): Date in format DD.MM.YYYY
        time_str (str): Time in format HH:MM
        weight (float): Weight in grams
        rssi (int): RSSI value
        snr (float): SNR value
    """
    try:
        # Parse date and time
        timestamp = datetime.strptime(f"{date_str} {time_str}", "%d.%m.%Y %H:%M")

        # Connect to the database
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()

        # Generate random values for fields not specified
        packets = random.randint(1, 5)
        counter = random.randint(1, 1000)
        message = f"PARCEL DELIVERY #{counter}"

        # Format timestamp for SQLite
        timestamp_str = timestamp.isoformat()

        # Insert data into database
        cursor.execute('''
        INSERT INTO lora_messages (
            timestamp, message, rssi, snr, packets, weight, counter
        )
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (
            timestamp_str,
            message,
            rssi,
            snr,
            packets,
            weight,
            counter
        ))

        # Commit and close
        conn.commit()
        conn.close()

        print(f"Successfully added sample: {timestamp_str}")
        print(f"Weight: {weight}g, RSSI: {rssi}, SNR: {snr}")
        print(f"Message: {message}, Packets: {packets}, Counter: {counter}")

        return True
    except Exception as e:
        print(f"Error inserting sample: {e}")
        return False

if __name__ == "__main__":
    # Set up command line argument parsing
    parser = argparse.ArgumentParser(description='Insert a single data point into the LoRa database.')

    parser.add_argument('date', help='Date in format DD.MM.YYYY')
    parser.add_argument('time', help='Time in format HH:MM')
    parser.add_argument('weight', type=float, help='Weight in grams')
    parser.add_argument('rssi', type=int, help='RSSI value')
    parser.add_argument('snr', type=float, help='SNR value')

    args = parser.parse_args()

    # Insert the sample
    insert_single_sample(
        args.date,
        args.time,
        args.weight,
        args.rssi,
        args.snr
    )
