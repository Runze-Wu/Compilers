import os
import subprocess
correct, total = 0, 0


def get_output(res_list):
    err = set()
    for res in res_list:
        if 'Error' not in res:
            break
        line = res.strip().split(' ')[5].strip(':')
        err.add(int(line))
    return err


def result_same(res_list1, res_list2):
    err1 = get_output(res_list1)
    err2 = get_output(res_list2)
    if len(err1) != len(err2):
        return False
    if len(err1.difference(err2)):
        return False
    return True


subprocess.call(
    args=['rm -rf /home/wrz/compilers/lab1/Lab/Test/*.cmm'], shell=True)
subprocess.call(
    args=['rm -rf /home/wrz/compilers/lab1/Lab/Test/*.out'], shell=True)
input_list = subprocess.getoutput('find ./tests/*.cmm').split('\n')

for file_name in input_list:
    total += 1
    flag = True

    std_file_name = file_name[:-4]+'.out'
    std_file = open(std_file_name)
    std_res_list = std_file.readlines()
    res_list = subprocess.getoutput(
        '/home/wrz/compilers/lab1/Lab/Code/parser '+file_name).split('\n')
    print('/home/wrz/compilers/lab1/Lab/Code/parser ' + file_name)
    if result_same(std_res_list, res_list) == False:
        print("test error")
        subprocess.call(args=['cp', file_name,
                              '/home/wrz/compilers/lab1/Lab/Test'])
        subprocess.call(args=['cp', std_file_name,
                              '/home/wrz/compilers/lab1/Lab/Test'])
        print(sorted(list(get_output(std_res_list))))
        print(sorted(list(get_output(res_list))))
    else:
        correct += 1
    # if correct < total:
    #     break
print(correct, total)
