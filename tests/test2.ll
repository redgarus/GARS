@out = private unnamed_addr constant [12 x i8] c"Output: %i\0A\00", align 1

declare i64 @printf(ptr, ...)

define i64 @print(i64 %0) {
entry:
  %calltmp = call i64 (ptr, ...) @printf(ptr @out, i64 %0)
  ret i64 0
}

define i64 @main() {
entry:
  %arrtemp = alloca [3 x i64], align 8

  %0 = getelementptr inbounds [3 x i64], ptr %arrtemp, i64 0, i64 0
  %1 = getelementptr inbounds [3 x i64], ptr %arrtemp, i64 0, i64 1
  %2 = getelementptr inbounds [3 x i64], ptr %arrtemp, i64 0, i64 2
  store i64 1, ptr %0, align 4
  store i64 2, ptr %1, align 4
  store i64 3, ptr %2, align 4
  
  %arrloadtemp = load [3 x i64], ptr %arrtemp, align 4
  
  %a = alloca [3 x i64], align 8
  
  store [3 x i64] %arrloadtemp, ptr %a, align 4
  
  %gep = getelementptr inbounds [3 x i64], ptr %a, i64 0, i64 0

  store i64 43, ptr %gep, align 4

  %3 = load i64, ptr %gep, align 4
  %calltmp = call i64 @print(i64 %3)
  ret i64 0
}
