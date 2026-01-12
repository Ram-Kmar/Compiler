import os
import subprocess
import sys

# Define test cases: (filename, expected_exit_code)
tests = [
    ("test.hy", 5),
    ("test2.hy", 5),
    ("test4.hy", 1),
    ("test_fib.hy", 55),
    ("test_func.hy", 30),
    ("test_print.hy", 0),
    ("test_logic.hy", 42),
]

COMPILER_PATH = "./build/compiler"
ASM_OUTPUT = "out.s"
EXECUTABLE = "./test_bin"

def run_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result

def main():
    if not os.path.exists(COMPILER_PATH):
        print(f"Error: Compiler not found at {COMPILER_PATH}. Please build it first.")
        sys.exit(1)

    passed = 0
    failed = 0

    print(f"{'Test File':<20} | {'Status':<10} | {'Expected':<10} | {'Actual':<10}")
    print("-" * 60)

    for filename, expected in tests:
        if not os.path.exists(filename):
            print(f"{filename:<20} | {'MISSING':<10} | {expected:<10} | {'-':<10}")
            failed += 1
            continue

        # 1. Compile .hy to .s
        compile_cmd = f"{COMPILER_PATH} {filename}"
        compile_res = run_command(compile_cmd)
        
        if compile_res.returncode != 0:
            print(f"{filename:<20} | {'COMPILE ERR':<10} | {expected:<10} | {'-':<10}")
            print(f"  Compiler Error: {compile_res.stderr}")
            failed += 1
            continue

        # 2. Assemble .s to executable
        # Using clang for macOS (Darwin)
        assemble_cmd = f"clang -o {EXECUTABLE} {ASM_OUTPUT}"
        assemble_res = run_command(assemble_cmd)

        if assemble_res.returncode != 0:
            print(f"{filename:<20} | {'ASM ERR':<10} | {expected:<10} | {'-':<10}")
            print(f"  Assembler Error: {assemble_res.stderr}")
            failed += 1
            continue

        # 3. Run the executable
        run_res = run_command(EXECUTABLE)
        actual = run_res.returncode

        if actual == expected:
            print(f"{filename:<20} | {'PASS':<10} | {expected:<10} | {actual:<10}")
            passed += 1
        else:
            print(f"{filename:<20} | {'FAIL':<10} | {expected:<10} | {actual:<10}")
            failed += 1

    print("-" * 60)
    print(f"Results: {passed} Passed, {failed} Failed")
    
    # Cleanup
    if os.path.exists(ASM_OUTPUT):
        os.remove(ASM_OUTPUT)
    if os.path.exists(EXECUTABLE):
        os.remove(EXECUTABLE)

    if failed > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
