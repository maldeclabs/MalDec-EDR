#!/usr/bin/python3

from subprocess import Popen
from regs import *
from ptrace import *
import argparse
from syscall_table import *

def main():
    try:
        parser = argparse.ArgumentParser(description="Script to trace system calls and analyze each one using a statistical calculation algorithm")

        # Add arguments
        parser.add_argument('-p', '--pid', type=int, help='PID of the running process')
        parser.add_argument('-t', '--time', type=int, help='Time interval collect syscalls')
        parser.add_argument('-c', '--count', type=int, help='Number of samples')

        # Parse command line arguments
        args = parser.parse_args()

        # Access argument values
        pid = args.pid
        count = args.count
        time = args.time

        # Program logic based on arguments
        if pid is not None and count is not None and time is not None:
            print(f"Analyzer pid={pid} time={time} count={count}\n")
            analyzer(pid, time, count)
        
        else:
            print("No arguments provided. Use -h or --help for help.")

    except KeyboardInterrupt:
        print("\nOperation interrupted by the user.")

if __name__ == "__main__":
    main()