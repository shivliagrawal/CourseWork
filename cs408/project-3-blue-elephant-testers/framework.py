import time
import sys
import os
import argparse
import subprocess
import telnetlib
from pathlib import Path


def normalize_output(output):
    """
    Normalizes dynamic values in server output for comparison.
    """
    # Replace dynamic parts of the output with placeholders
    output = output.replace("127,0,0,1,", "127,0,0,1,*")
    output = output.replace("/absolute/path", "/placeholder_path")
    return output.strip()


def run_test_case(binary, config_file, test_commands, expected_output, verbose):
    """
    Runs a single test case with the specified binary, configuration file, and commands.
    Compares server output with expected output file.
    """
    execute_line = f'./{binary} -c {config_file} -D'

    # Start the BFTPD server
    proc = subprocess.Popen(execute_line, shell=True)
    time.sleep(2)

    if proc.poll() is not None:
        print("BFTPD failed to start. Try changing your port.")
        return False

    # Dictionary to manage multiple client connections
    clients = {}
    full_actual_output = []  # Store actual output for comparison
    data_tn = None  # For handling the data connection

    try:
        all_pass = True

        for idx, command in enumerate(test_commands):
            # Parse client number and command
            parts = command.strip().split(":", 1)
            if len(parts) != 2:
                print(f"Invalid command format at line {idx + 1}: {command}")
                all_pass = False
                continue

            client_num, actual_command = parts
            client_num = int(client_num.strip())

            # Establish a connection for the client if it doesn't exist
            if client_num not in clients:
                host = 'localhost'
                port = 14703  # Use the port specified in your bftpd.conf
                try:
                    clients[client_num] = telnetlib.Telnet(host, port)
                    print(f"Client {client_num}: Connected to {host} on port {port}")

                    # Ignore the first line (welcome message)
                    first_response = clients[client_num].read_until(b'\n', timeout=5).decode('ascii').strip()
                    print(f"Ignoring first server response: {first_response}")

                except Exception as e:
                    print(f"Client {client_num}: Error connecting to BFTPD server: {e}")
                    all_pass = False
                    continue

            # Send the command and read the response
            tn = clients[client_num]
            tn.write((actual_command + '\r\n').encode('ascii'))
            response = tn.read_until(b'\n', timeout=5).decode('ascii').strip()
            print(f"Response received: {response}")

            # Add actual response to the list for comparison
            full_actual_output.append(response)

        # Normalize and compare actual vs. expected output
        full_actual_output_normalized = [normalize_output(line) for line in full_actual_output]
        expected_output_normalized = [normalize_output(line) for line in expected_output]

        if full_actual_output_normalized != expected_output_normalized:
            all_pass = False
            if verbose:
                print("\n--- Differences Detected ---")
                print("Expected Output:")
                print("\n".join(expected_output_normalized))
                print("Actual Output:")
                print("\n".join(full_actual_output_normalized))
                print("--- End of Differences ---\n")

    finally:
        # Close all client connections
        for client_num, tn in clients.items():
            tn.close()
            print(f"Client {client_num}: Connection closed.")

        # Close the data connection if open
        if data_tn:
            data_tn.close()
            print("Data connection closed.")

        # Terminate the BFTPD process
        proc.terminate()
        proc.wait()

    return all_pass


def run_all_tests(binary, verbose):
    """
    Runs all test cases from the testcases directory.
    """
    testcases_path = Path("testing_environment/testcases")
    test_files = testcases_path.glob("test_input_*.txt")

    for test_file in test_files:
        with open(test_file, 'r') as f:
            # Read configuration file and test commands
            config_file = "testing_environment/configurations/" + f.readline().strip()
            test_commands = [line.strip() for line in f.readlines()]

        # Load expected output
        expected_output_file = test_file.parent / test_file.name.replace("input", "output")
        if not expected_output_file.exists():
            print(f"Expected output file not found for {test_file.name}")
            continue

        with open(expected_output_file, 'r') as f:
            expected_output = [line.strip() for line in f.readlines()]

        print(f"Running test case: {test_file.name}")
        success = run_test_case(binary, config_file, test_commands, expected_output, verbose)

        if success:
            print(f"Test case {test_file.name}: PASSED")
        else:
            print(f"Test case {test_file.name}: FAILED")
        print("-" * 40)


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description="test cases for bftpd server")
    parser.add_argument('-p', action="store", dest="binary", required=True, help="Path to the bftpd binary")
    parser.add_argument('-f', action="store", dest="file", help="Run a single test case file")
    parser.add_argument('-v', action="store_true", dest="verbose", help="Enable verbose output")

    args = parser.parse_args()

    if args.file:
        # Run a single test case
        with open(args.file, 'r') as f:
            config_file = "testing_environment/configurations/" + f.readline().strip()
            test_commands = [line.strip() for line in f.readlines()]

        # Load expected output
        expected_output_file = Path(args.file).parent / Path(args.file).name.replace("input", "output")
        if not expected_output_file.exists():
            print(f"Expected output file not found for {args.file}")
            sys.exit(2)

        with open(expected_output_file, 'r') as f:
            expected_output = [line.strip() for line in f.readlines()]

        print(f"Running single test case: {args.file}")
        success = run_test_case(args.binary, config_file, test_commands, expected_output, args.verbose)

        if success:
            print("Test case PASSED")
            sys.exit(0)
        else:
            print("Test case FAILED")
            sys.exit(1)

    else:
        # Run all test cases
        print("Running all test cases...")
        run_all_tests(args.binary, args.verbose)

