import os

# Define the binaries and corresponding test cases
test_matrix = {
    "./bin/bftpdBug0": ["./testing_environment/testcases/test_input_1.txt", "./testing_environment/testcases/test_input_2.txt"],
    "./bin/bftpdBug1": ["./testing_environment/testcases/test_input_3.txt", "./testing_environment/testcases/test_input_4.txt"],
    "./bin/bftpdBug2": ["./testing_environment/testcases/test_input_5.txt", "./testing_environment/testcases/test_input_8.txt"],
    "./bin/bftpdBug3": ["./testing_environment/testcases/test_input_6.txt", "./testing_environment/testcases/test_input_9.txt"],
    "./bin/bftpdBug4": ["./testing_environment/testcases/test_input_3.txt", "./testing_environment/testcases/test_input_4.txt", "./testing_environment/testcases/test_input_7.txt"],
    "./bin/bftpdBug5": ["./testing_environment/testcases/test_input_5.txt", "./testing_environment/testcases/test_input_8.txt"],
    "./bin/bftpdBug6": ["./testing_environment/testcases/test_input_1.txt", "./testing_environment/testcases/test_input_9.txt"],
    "./bin/bftpdBug7": ["./testing_environment/testcases/test_input_3.txt", "./testing_environment/testcases/test_input_6.txt"],
    "./bin/bftpdBug8": ["./testing_environment/testcases/test_input_2.txt", "./testing_environment/testcases/test_input_8.txt"],
    "./bin/bftpdBug9": ["./testing_environment/testcases/test_input_1.txt", "./testing_environment/testcases/test_input_7.txt"],
}

# Path to the configuration file
config_path = "./testing_environment/configurations/bftpd.conf"

# Function to update the configuration file
def modify_conf(user_limit=None):
    """
    Dynamically update the bftpd.conf file with specific options.
    """
    try:
        with open(config_path, "r") as f:
            lines = f.readlines()

        # Modify USERLIMIT_SINGLEUSER if needed
        if user_limit is not None:
            for i, line in enumerate(lines):
                if line.strip().startswith("USERLIMIT_SINGLEUSER"):
                    lines[i] = f"  USERLIMIT_SINGLEUSER={user_limit}\n"

        # Write the updated configuration back
        with open(config_path, "w") as f:
            f.writelines(lines)

        print(f"Configuration file updated: USERLIMIT_SINGLEUSER={user_limit}")
    except Exception as e:
        print(f"Error modifying configuration file: {e}")

# Iterate through each binary and test case
for binary, test_cases in test_matrix.items():
    for test_case in test_cases:
        # Special handling for test_input_7.txt
        if "test_input_7.txt" in test_case:
            modify_conf(user_limit=1)
        else:
            modify_conf(user_limit=0)  # Reset to default

        # Run the test
        print(f"Testing {binary} with {test_case}...")
        result = os.system(f"python3 framework.py -p {binary} -f {test_case} -v")
        if result == 0:
            print(f"SUCCESS: {binary} passed with {test_case}")
        else:
            print(f"FAILURE: {binary} failed with {test_case}")
    print("-" * 60)

# Reset the configuration to default after all tests
modify_conf(user_limit=0)
