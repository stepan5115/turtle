#!/bin/bash

EXEC=./turtle

LD_LIB=/app/antlr4-runtime/lib
export LD_LIBRARY_PATH=$LD_LIB

GREEN="\033[0;32m"
RED="\033[0;31m"
YELLOW="\033[1;33m"
BLUE="\033[0;34m"
NC="\033[0m"

PASS=0
FAIL=0

echo -e "${BLUE}=== PREPARING PROJECT ===${NC}"
make all || { echo -e "${RED}Сборка не удалась${NC}"; exit 1; }

run_test() {
    local file=$1
    local expected=$2

    timeout 2s make run INPUT_FILE="$file" FLAGS=--testing > /dev/null 2>&1
    STATUS=$?

    RESULT=""

    if [ $STATUS -eq 0 ]; then
        RESULT="OK"
    elif [ $STATUS -eq 124 ]; then
        RESULT="TIMEOUT"
    elif [ $STATUS -eq 139 ]; then
        RESULT="SEGFAULT"
    else
        RESULT="ERROR"
    fi

    if [[ "$expected" == "valid" && "$RESULT" == "OK" ]]; then
        echo -e "${GREEN}[PASS]${NC} $file"
        ((PASS++))
    elif [[ "$expected" == "invalid" && "$RESULT" != "OK" ]]; then
        echo -e "${GREEN}[PASS]${NC} $file (${RESULT})"
        ((PASS++))
    else
        echo -e "${RED}[FAIL]${NC} $file (got: $RESULT)"
        ((FAIL++))
    fi
}

echo -e "${BLUE}=== VALID TESTS ===${NC}"
for f in tests/valid/*.turtle; do
    run_test "$f" "valid"
done

echo -e "\n${BLUE}=== INVALID TESTS ===${NC}"
for f in tests/invalid/*.turtle; do
    run_test "$f" "invalid"
done

echo -e "\n======================"
echo -e "PASS: ${GREEN}$PASS${NC}"
echo -e "FAIL: ${RED}$FAIL${NC}"

if [ $FAIL -ne 0 ]; then
    exit 1
fi