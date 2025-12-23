#!/usr/bin/env python3
"""
Diagnostic test to check individual score components
"""

import serial
import time
import sys

def send_command(ser, cmd, wait_time=0.3):
    """Send command and capture response"""
    ser.write((cmd + '\n').encode())
    time.sleep(wait_time)
    response = ""
    while ser.in_waiting:
        response += ser.read(ser.in_waiting).decode('utf-8', errors='ignore')
        time.sleep(0.05)
    return response

def test_voice_leading_context(port='/dev/tty.usbmodem183415201', baud=115200):
    try:
        ser = serial.Serial(port, baud, timeout=1)
        time.sleep(2)
        print("Connected to device")
    except Exception as e:
        print(f"Failed to connect: {e}")
        sys.exit(1)
    
    print("="*70)
    print("DIAGNOSTIC: Voice-Leading Context Testing")
    print("="*70)
    
    # Reset
    print("\n1. RESET")
    print(send_command(ser, "TEST RESET", 0.5))
    
    # Set context to C major
    print("\n2. SET C MAJOR CONTEXT")
    print(send_command(ser, "TEST GLOBAL root=0 degree=0 theory=0 vl=0.5 energy=0.5"))
    
    # Rank with NO chords (neutral voice-leading)
    print("\n3. RANK WITH NO CHORDS (currentChord=SENTINEL 255)")
    resp = send_command(ser, "TEST RANK")
    print(resp)
    print("\n   TOP 3 EXPECTED: D, F (equal), E")
    
    # Add C major
    print("\n4. ADD C MAJOR (root=0)")
    print(send_command(ser, "TEST CHORD slot=0 root=0 type=0 beats=16"))
    
    # Rank with C major current (should show distance-based scores)
    print("\n5. RANK WITH C MAJOR CURRENT (currentChord=C/0)")
    resp = send_command(ser, "TEST RANK")
    lines = resp.split('\n')
    for line in lines[:15]:
        print(line)
    
    # Extract top scores and analyze
    print("\n   ANALYSIS:")
    print("   - F is 5 semitones from C (VL preference=0.5, distance=5)")
    print("   - G is 7 semitones from C (but shortest=5)")
    print("   - D is 2 semitones from C (VL should favor this at 0.5)")
    print("   - Expected: G(0.86), then F(0.92), or D higher")
    
    # Add F minor (move current to slot 1)
    print("\n6. ADD F MINOR (root=5) at SLOT 1")
    print(send_command(ser, "TEST CHORD slot=1 root=5 type=1 beats=8"))
    
    # Rank with F minor current (should show different VL distances)
    print("\n7. RANK WITH F MINOR CURRENT (currentChord=F/5)")
    resp = send_command(ser, "TEST RANK")
    lines = resp.split('\n')
    for line in lines[:15]:
        print(line)
    
    print("\n   ANALYSIS:")
    print("   - If rankings are IDENTICAL to step 5, currentChord is NOT changing")
    print("   - If rankings CHANGE, currentChord is correctly updating")
    print("   - Expected change: Bb(10) should move up (2 semitones from F=5)")
    
    # Check STATE to verify current slot
    print("\n8. VERIFY CURRENT CHORD STATE")
    print(send_command(ser, "TEST STATE"))
    
    ser.close()

if __name__ == "__main__":
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--port", default="/dev/tty.usbmodem183415201")
    parser.add_argument("--baud", type=int, default=115200)
    args = parser.parse_args()
    test_voice_leading_context(args.port, args.baud)
