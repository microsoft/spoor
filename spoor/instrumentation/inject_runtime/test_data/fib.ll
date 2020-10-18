; // Source C++
; auto Fibonacci(int n) -> int {
;   if (n < 2) return n;
;   return Fibonacci(n - 1) + Fibonacci(n - 2);
; }
;
; auto main() -> int { return Fibonacci(7); }

define i32 @_Z9Fibonaccii(i32 %0) {
  %2 = icmp slt i32 %0, 2
  br i1 %2, label %9, label %3
3:
  %4 = add nsw i32 %0, -1
  %5 = tail call i32 @_Z9Fibonaccii(i32 %4)
  %6 = add nsw i32 %0, -2
  %7 = tail call i32 @_Z9Fibonaccii(i32 %6)
  %8 = add nsw i32 %7, %5
  ret i32 %8
9:
  ret i32 %0
}

define i32 @main() {
  %1 = tail call i32 @_Z9Fibonaccii(i32 7)
  ret i32 %1
}
