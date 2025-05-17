
#!/usr/bin/env python3
import json
import serial
import sqlite3
import time
import logging
from datetime import datetime

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler('/var/log/lora_receiver.log'),
        logging.StreamHandler()
    ]
)

# Database setup
DB_PATH = '/var/lib/grafana/lora_data.db'

def setup_database():
    """Create SQLite database and table if it doesn't exist."""
    try:
        conn = sqlite3.connect(DB_PATH)
        cursor = conn.cursor()

        cursor.execute('''
        CREATE TABLE IF NOT EXISTS lora_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            message TEXT NOT NULL,
            rssi INTEGER NOT NULL,
            snr REAL NOT NULL,
            packets INTEGER NOT NULL,
            weight REAL,
            counter INTEGER
        )
        ''')

        conn.commit()
        logging.info("Database setup complete")
        return conn
    except Exception as e:
        logging.error(f"Database setup failed: {e}")
        raise

def process_message(conn, data):
    """Process and store the received message in the database."""
    try:
        json_data = json.loads(data)

        # Extract data from JSON
        message = json_data.get('message', '')
        rssi = json_data.get('rssi', 0)
        snr = json_data.get('snr', 0.0)
        packets = json_data.get('packets', 0)
        weight = json_data.get('weight', 0.0)
        counter = json_data.get('counter', 0)

        # Get current timestamp
        timestamp = datetime.now().isoformat()

        # Insert data into database
        cursor = conn.cursor()
        cursor.execute('''
        INSERT INTO lora_messages (timestamp, message, rssi, snr, packets, weight, counter)
        VALUES (?, ?, ?, ?, ?, ?, ?)
        ''', (timestamp, message, rssi, snr, packets, weight, counter))

        # Log the detection based on the weight reading
        if weight > 0:
            logging.info(f"PARCEL DETECTED! Weight: {weight}g at {timestamp}")

        conn.commit()
        logging.info(f"Stored message: {message}, RSSI: {rssi}, SNR: {snr}, Weight: {weight}g, Counter: {counter}")
    except Exception as e:
        logging.error(f"Error processing message: {e}")

def main():
    """Main function to read from serial port and process data."""
    serial_port = '/dev/ttyAMA0'
    baud_rate = 115200

    logging.info(f"Starting LoRa receiver on {serial_port} at {baud_rate} baud")

    # Set up the database
    conn = setup_database()

    try:
        # Open serial port
        ser = serial.Serial(serial_port, baud_rate, timeout=1)
        logging.info("Serial port opened successfully")

        # Main loop
        while True:
            if ser.in_waiting > 0:
                # Read line from serial port
                line = ser.readline().decode('utf-8', errors='replace').strip()
                if line and line.startswith('{') and line.endswith('}'):
                    logging.info(f"Received data: {line}")
                    process_message(conn, line)

            # Small delay to reduce CPU usage
            time.sleep(0.01)

    except KeyboardInterrupt:
        logging.info("Program terminated by user")
    except Exception as e:
        logging.error(f"Unexpected error: {e}")
    finally:
        if 'conn' in locals():
            conn.close()
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()
