"""
Author: Yash Pathak [Github: vindicta07]
Email: yashpradeeppathak@gmail.com
Description: Block extraction algorithm for counting blocks to remove
"""

def count_blocks_to_remove(matrix, N, M, K):
    blocks_to_remove = set()

    # First, find all the positions of the block K
    positions_of_k = []
    for i in range(N):
        for j in range(M):
            if matrix[i][j] == K:
                positions_of_k.append((i, j))

    # Now, for each position of K, check the blocks above it
    for i, j in positions_of_k:
        # Check all rows above the current position
        for row in range(i):
            block_above = matrix[row][j]
            blocks_to_remove.add(block_above)  # Add the block above in the same column

    # Remove K from the set if it exists, since we are only interested in blocks above it
    blocks_to_remove.discard(K)

    # Now we check if any blocks in other columns are overlapping with K and are above it
    for i, j in positions_of_k:
        # Check all columns
        for col in range(M):
            if col != j:  # Only check other columns
                for row in range(i):  # Check all rows above the current row of K
                    if matrix[row][col] != K:  # Only consider blocks that are not K
                        # If the block is directly above K's row, we need to check if it overlaps
                        if matrix[row][col] == K or (row < i and matrix[row][col] != 0):
                            blocks_to_remove.add(matrix[row][col])
    
    return len(blocks_to_remove)

N, M = map(int, input().split())
matrix = [list(map(int, input().split())) for _ in range(N)]
K = int(input())

result = count_blocks_to_remove(matrix, N, M, K)
print(result)