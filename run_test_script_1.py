#!/usr/bin/env python3
"""
Chord Sequencer Test Automation Script
Sends TEST commands to Teensy via serial and captures output
"""

import serial
import time
import sys

def send_test_command(ser, cmd, wait_time=0.5):
    """Send a single TEST command and capture response"""
    print(f"\n{'='*70}")
    print(f"COMMAND: {cmd}")
    print(f"{'='*70}")
    
    ser.write((cmd + '\n').encode())
    time.sleep(wait_time)
    
    response = ""
    while ser.in_waiting:
        response += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        time.sleep(0.1)
    
    print("OUTPUT:")
    print(response)
    return response

def run_test_script_1(port='/dev/ttyUSB0', baud=115200):
    """
    TEST SCRIPT #1 — SIMPLE CIRCLE-OF-FIFTHS SEED
    Tests: global parameters, ranking from C major, F minor selection, modal mixture
    """
    
    try:
        ser = serial.Serial(port, baud, timeout=1)
        time.sleep(2)  # Wait for Teensy to be ready
        print("Connected to device at", port)
    except Exception as e:
        print(f"Failed to connect: {e}")
        print(f"Try: ls /dev/tty.* or ls /dev/ttyUSB*")
        sys.exit(1)
    
    # Define test commands
    commands = [
        ("TEST RESET", "Reset sequencer to clean state"),
        ("TEST STATE", "Verify reset: should show chords=0"),
        ("TEST GLOBAL root=0 degree=0 theory=0 vl=0.5 energy=0.5", "Set C major context"),
        ("TEST RANK", "Baseline ranking in C major (no chords yet)"),
        ("TEST CHORD slot=0 root=0 type=0 beats=16", "Add C major as first chord"),
        ("TEST STATE", "Verify C major added, chordCount=1"),
        ("TEST RANK", "Rank candidates from C major (G, F, Am should be high)"),
        ("TEST CHORD slot=1 root=5 type=1 beats=8", "Add F minor as second chord (modal mixture)"),
        ("TEST STATE", "Verify F minor added, chordCount=2"),
        ("TEST RANK", "Rank candidates from F minor (Bb, Ab should appear higher now)"),
        ("TEST CHORD slot=2 root=10 type=0 beats=8", "Add Bb major as third chord"),
        ("TEST STATE", "Final state: 3 chords in progression"),
    ]
    
    print("\n" + "="*70)
    print("TEST SCRIPT #1 — SIMPLE CIRCLE-OF-FIFTHS SEED")
    print("="*70)
    print("Testing: global parameters, ranking, modal mixture effects")
    print()
    
    results = []
    for cmd, description in commands:
        print(f"\n[{description}]")
        response = send_test_command(ser, cmd)
        results.append((cmd, description, response))
        time.sleep(0.2)
    
    # Summary
    print("\n" + "="*70)
    print("TEST SUMMARY")
    print("="*70)
    for i, (cmd, desc, resp) in enumerate(results, 1):
        status = "✓" if "OK" in resp or "STATE" in resp or "RANK" in resp else "?"
        print(f"{i}. {status} {desc}")
        print(f"   CMD: {cmd}")
    
    ser.close()
    print("\nTest complete. Check output above for validation.")

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser(description="Chord Sequencer Test Automation")
    parser.add_argument("--port", default="/dev/ttyUSB0", help="Serial port (default: /dev/ttyUSB0)")
    parser.add_argument("--baud", type=int, default=115200, help="Baud rate (default: 115200)")
    args = parser.parse_args()
    
    run_test_script_1(args.port, args.baud)
