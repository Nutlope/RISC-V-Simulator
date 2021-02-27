def shift(val, scale):
    return val << scale

def matrix_opr(mat, out):
    count = 0
    for i in range(4):
        out[i] = 0
        for j in range(4):
            out[i] += shift(mat[i*4+j], i)
            count += 1
    return count
    
out = [0,0,0,0]
mat = list(range(16))

print(matrix_opr(mat, out))