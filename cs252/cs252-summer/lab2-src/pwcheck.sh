#!/bin/bash

# DO NOT REMOVE THE FOLLOWING LINES
git add $0 >> .local.git.out
git commit -a -m "Lab 2 commit" >> .local.git.out
git push >> .local.git.out || echo

# Function to calculate the password score
calculate_score() {
    local PASSWORD="$1"
    local COUNT=0

    # Initial score based on the length of the password
    COUNT=${#PASSWORD}

    # Bonus points for special characters
    if [[ "$PASSWORD" =~ [#$+%@] ]]; then
        let COUNT+=5
    fi

    # Bonus points for digits
    if [[ "$PASSWORD" =~ [0-9] ]]; then
        let COUNT+=5
    fi

    # Bonus points for letters
    if [[ "$PASSWORD" =~ [A-Za-z] ]]; then
        let COUNT+=5
    fi

    # Penalty for repeated characters
    if [[ "$PASSWORD" =~ ([A-Za-z0-9])\1+ ]]; then
        let COUNT-=10
    fi

    # Penalty for consecutive lowercase letters
    if [[ "$PASSWORD" =~ [a-z]{3,} ]]; then
        let COUNT-=3
    fi

    # Penalty for consecutive uppercase letters
    if [[ "$PASSWORD" =~ [A-Z]{3,} ]]; then
        let COUNT-=3
    fi

    # Penalty for consecutive digits
    if [[ "$PASSWORD" =~ [0-9]{3,} ]]; then
        let COUNT-=3
    fi

    # Print the password and its score
    printf "%-20s %5d\n" "$PASSWORD" "$COUNT"
}

# Function to print usage information
print_usage() {
    echo "Usage: $0 [-f passwordFile] password1 password2 password3 ..."
    exit 1
}

# Initialize an array to store passwords
PASSWORDS=()

# Process command-line arguments
while (( "$#" )); do
    case "$1" in
        -f)
            if [ -n "$2" ] && [ -f "$2" ]; then
                while IFS= read -r line || [[ -n "$line" ]]; do
                    for word in $line; do
                        PASSWORDS+=("$word")
                    done
                done < "$2"
                shift 2
            else
                echo "Error: Missing or invalid file after -f"
                exit 1
            fi
            ;;
        *)
            PASSWORDS+=("$1")
            shift
            ;;
    esac
done

# Check if no passwords are provided
if [ ${#PASSWORDS[@]} -eq 0 ]; then
    print_usage
fi

# Print header
echo -e "Password\t\tScore"
echo "--------------------   -----"

# Calculate score for each password
for PASSWORD in "${PASSWORDS[@]}"; do
    if [ ${#PASSWORD} -gt 32 ] || [ ${#PASSWORD} -lt 6 ]; then
        printf "%-20s %5s\n" "$PASSWORD" "Error: Password length invalid."
    else
        calculate_score "$PASSWORD"
    fi
done

