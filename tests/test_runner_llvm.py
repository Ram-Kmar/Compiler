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

LLVM_OUTPUT = os.path.join(SCRIPT_DIR, "out.ll")

EXECUTABLE_NAME = "test_bin_llvm.exe" if os.name == 'nt' else "test_bin_llvm"
EXECUTABLE = os.path.join(SCRIPT_DIR, EXECUTABLE_NAME)

def run_command(command):
    result = subprocess.run(command, shell=True, capture_output=True, text=True)
    return result

def main():
    # On Windows, build path might be slightly different depending on generator (e.g. Debug/Release folders)
    # But usually cmake puts it directly in build or build/Debug. 
    # We will stick to the default assumption for now.
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

    passed = 0
    failed = 0

    print(f"{ 'Test File':<20} | { 'Status':<10} | { 'Expected':<10} | { 'Actual':<10}")
    print("-" * 60)

    for filename, expected in tests:
        filepath = os.path.join(SCRIPT_DIR, filename)
        if not os.path.exists(filepath):
            print(f"{filename:<20} | {'MISSING':<10} | {expected:<10} | {'-':<10}")
            failed += 1
            continue

        # 1. Compile .hy to .ll
        # We run the command in SCRIPT_DIR so out.ll is generated there
        compile_cmd = f"{COMPILER_PATH} {filepath}"
        compile_res = subprocess.run(compile_cmd, shell=True, capture_output=True, text=True, cwd=SCRIPT_DIR)
        
        if compile_res.returncode != 0:
            print(f"{filename:<20} | {'COMPILE ERR':<10} | {expected:<10} | {'-':<10}")
            print(f"  Compiler Error: {compile_res.stderr}")
            failed += 1
            continue

        if not os.path.exists(LLVM_OUTPUT):
             print(f"{filename:<20} | {'NO LLVM OUT':<10} | {expected:<10} | {'-':<10}")
             failed += 1
             continue

        # 2. Compile .ll to executable using clang
        # Windows requires linking against legacy_stdio_definitions for printf
        link_flags = ""
        if os.name == 'nt':
            link_flags = "-llegacy_stdio_definitions -lucrt"

        # -Wno-override-module suppresses warnings about target triple mismatch (e.g. macos vs windows)
        assemble_cmd = f"clang -Wno-override-module -o {EXECUTABLE} {LLVM_OUTPUT} {link_flags}"
        assemble_res = run_command(assemble_cmd)

        if assemble_res.returncode != 0:
            print(f"{filename:<20} | {'CLANG ERR':<10} | {expected:<10} | {'-':<10}")
            print(f"  Clang Error: {assemble_res.stderr}")
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
    if os.path.exists(LLVM_OUTPUT):
        os.remove(LLVM_OUTPUT)
    if os.path.exists(os.path.join(SCRIPT_DIR, "out.s")):
        os.remove(os.path.join(SCRIPT_DIR, "out.s"))
    if os.path.exists(EXECUTABLE):
        os.remove(EXECUTABLE)

    if failed > 0:
        sys.exit(1)

if __name__ == "__main__":
    main()
