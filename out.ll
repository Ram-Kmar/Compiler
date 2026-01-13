declare i32 @printf(i8*, ...)
@.str = private unnamed_addr constant [4 x i8] c"%d\0Adefine i32 @fib(i32 %n) {
entry:
  %n.addr = alloca i32
  store i32 %n, i32* %n.addr
  %r0 = load i32, i32* %n.addr
  %r1 = icmp slt i32 %r0, 2
  br i1 %r1, label %L0, label %L1
L0:
  %r2 = load i32, i32* %n.addr
  ret i32 %r2
  br label %L2
L1:
  br label %L2
L2:
  %r3 = load i32, i32* %n.addr
  %r4 = sub i32 %r3, 1
  %r5 = call i32 @fib(i32 %r4)
  %r6 = load i32, i32* %n.addr
  %r7 = sub i32 %r6, 2
  %r8 = call i32 @fib(i32 %r7)
  %r9 = add i32 %r5, %r8
  ret i32 %r9
  ret i32 0
}

define i32 @main() {
entry:
  %r0 = call i32 @fib(i32 10)
  ret i32 %r0
  ret i32 0
}

