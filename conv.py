import sys

if len(sys.argv) < 2:
    print("no arg")
    sys.exit(0)

n = int(sys.argv[1])

sign = n >> 7
man  = (n & 0b111) | 0b1000
exp  = (n >> 3) & 0b1111

print(f"sign: {bin(sign)}, exp: {bin(exp)}, man: {bin(man)}")

#twoc
if exp & 0b1000:
    exp -= 0b10000

print(f"sign: {(sign)}, exp: {(exp)}, man: {(man)}")

res = (-1 if sign else 1) * man * (2 ** exp)
print(f"=> {res}")


