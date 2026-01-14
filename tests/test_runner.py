import os
import subprocess
import sys

# Get the directory where this script is located
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Define test cases: (filename, expected_exit_code)
tests = [
    ("test.hy", 5),
    ("test2.hy", 5),
    ("test4.hy", 1),
    ("test_fib.hy", 55),
    ("test_func.hy", 30),
    ("test_print.hy", 0),
    ("test_logic.hy", 42),
    ("test_for.hy", 10),
    ("test_ptr.hy", 20),
]

# Adjust paths for Windows if necessary
COMPILER_NAME = "compiler.exe" if os.name == 'nt' else "compiler"
COMPILER_PATH = os.path.join(SCRIPT_DIR, "../build", COMPILER_NAME)

ASM_OUTPUT = os.path.join(SCRIPT_DIR, "out.s")

EXECUTABLE_NAME = "test_bin.exe" if os.name == 'nt' else "test_bin"
EXECUTABLE = os.path.join(SCRIPT_DIR, EXECUTABLE_NAME)

def run_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result

def main():
    if not os.path.exists(COMPILER_PATH):
        # Fallback check for Windows Debug/Release folders
        if os.name == 'nt':
             debug_path = os.path.join(SCRIPT_DIR, "../build/Debug", COMPILER_NAME)
             if os.path.exists(debug_path):
                 globals()['COMPILER_PATH'] = debug_path
             else:
                 print(f"Error: Compiler not found at {COMPILER_PATH} or {debug_path}. Please build it first.")
                 sys.exit(1)
        else:
            print(f"Error: Compiler not found at {COMPILER_PATH}. Please build it first.")
            sys.exit(1)

    # Warning for non-ARM64 platforms
    # This is a heuristic check.
    import platform
    machine = platform.machine().lower()
    if 'arm' not in machine and 'aarch64' not in machine:
        print(f"WARNING: You are running on {machine}, but this test suite relies on ARM64 assembly.")
        print("It will likely fail. Please use test_runner_llvm.py instead.")
        print("-" * 60)

    passed = 0
    failed = 0

    print(f"{'Test File':<20} | {'Status':<10} | {'Expected':<10} | {'Actual':<10}")
    print("-" * 60)

    for filename, expected in tests:
        filepath = os.path.join(SCRIPT_DIR, filename)
        if not os.path.exists(filepath):
            print(f"{filename:<20} | {'MISSING':<10} | {expected:<10} | {'-':<10}")
            failed += 1
            continue

        # 1. Compile .hy to .s
        # We run the command in SCRIPT_DIR so out.s is generated there
        compile_cmd = f"\"{COMPILER_PATH}\" \"{filepath}\""
        compile_res = subprocess.run(compile_cmd, shell=True, capture_output=True, text=True, cwd=SCRIPT_DIR)
        
        if compile_res.returncode != 0:
            print(f"{filename:<20} | {'COMPILE ERR':<10} | {expected:<10} | {'-':<10}")
            print(f"  Compiler Error: {compile_res.stderr}")
            failed += 1
            continue

        # 2. Assemble .s to executable
        # Using clang for macOS (Darwin) or generic *nix
        assemble_cmd = f"clang -o \"{EXECUTABLE}\" \"{ASM_OUTPUT}\""
        assemble_res = run_command(assemble_cmd)

        if assemble_res.returncode != 0:
            print(f"{filename:<20} | {'ASM ERR':<10} | {expected:<10} | {'-':<10}")
            print(f"  Assembler Error: {assemble_res.stderr}")
            failed += 1
            continue

        # 3. Run the executable
        run_res = run_command(f"\"{EXECUTABLE}\"")
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
    if os.path.exists(os.path.join(SCRIPT_DIR, "out.ll")): # Cleanup out.ll too
        os.remove(os.path.join(SCRIPT_DIR, "out.ll"))
    if os.path.exists(EXECUTABLE):
        os.remove(EXECUTABLE)

    if failed > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
