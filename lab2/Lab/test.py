import subprocess
correct, total = 0, 0


def get_output(res_list):
    err = dict()
    for res in res_list:
        if 'Error' not in res:
            break
        res = res.strip().split(' ')
        if res[5].strip(':') not in err:
            err[res[5].strip(':')] = set()
        err[res[5].strip(':')].add(res[2])
    return err


def result_same(std_res, my_res):
    err_std = get_output(std_res)
    err_my = get_output(my_res)
    std_lines = set(err_std.keys())
    my_lines = set(err_my.keys())
    if len(std_lines.difference(my_lines)) or len(my_lines.difference(std_lines)):
        print("line diff")
        print(std_lines.difference(my_lines))
        print(my_lines.difference(std_lines))
        return False
    for line in std_lines:
        if len(err_std[line].difference(err_my[line])):
            print("type diff in "+line)
            print(err_std[line])
            print(err_my[line])
            print(err_std[line].difference(err_my[line]))
            return False

    return True


input_list = subprocess.getoutput('find ./Tests/*.cmm').split('\n')
err_file_list = []
for file_name in input_list:
    total += 1
    std_file_name = file_name[:-4]+'.log'
    std_file = open(std_file_name)
    std_res_list = std_file.readlines()
    res_list = subprocess.getoutput(
        './parser '+file_name).split('\n')
    print('./parser ' + file_name)
    if result_same(std_res_list, res_list) == False:
        print("test error")
        err_file_list.append(file_name)
    else:
        correct += 1
    # if correct < total:
    #     break
print(correct, total)
print(err_file_list)
