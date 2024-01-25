from pwn import *
from itertools import combinations
from tqdm import tqdm

context.log_level = "ERROR"


def test(data: bytes):
	r.sendline(data)
	return r.recvline()

b = [bytes([i]) for i in range(128)]
tests = list(map(lambda x: b''.join(x), combinations(b, 2)))

for case in tqdm(tests):
	try:
		r = remote("127.0.0.1", "6668")
		#print(case)
		res = test(case)
		assert b":@127.0.0.1 421  " in res and b" :Unknown command\r\n" in res

		r.close()
	except Exception as e:
		if b'\x00' not in case:
			print(e, case)
		r.close()
		try:
			print(res)
		except:
			pass

