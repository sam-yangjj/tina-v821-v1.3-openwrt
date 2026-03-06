import os
import subprocess
import argparse

# e.g. python modparse ./linux-5.15 5.15.78 .
parser = argparse.ArgumentParser(description="Parse Linux kernel Module Dependencies")
parser.add_argument("kerneldir", help="Path to the kernel build dir")
parser.add_argument("version", help="kernelversion")
parser.add_argument("--builtin", help="modules.builtin file path")
parser.add_argument("--module", help="module dir")
parser.add_argument("--input", help="module list file")
parser.add_argument("--output", help="output file")
parser.add_argument("--early", help="early insmod modules")
parser.add_argument("--earlyout", help="early module output file")
args = parser.parse_args()

kernelversion = args.version
kerneldir = args.kerneldir
builtinfile = None
builtin_list = []
inputfile = None
moddir = None
if args.input is not None:
    inputfile = args.input
else:
    moddir = args.module

if args.builtin is not None:
    builtinfile = args.builtin

outfile = None
if args.output is not None:
    outfile = args.output

earlymods = None
if args.output is not None:
    earlymods = args.early

earlyfile = None
if args.output is not None:
    earlyfile = args.earlyout

def is_buildin(module_name):
    if builtinfile == None:
        return False

    for item in builtin_list:
        if module_name == item:
            return True
    return False

def get_module_dependencies(module):
    try:
        cmd = "modprobe -S {} -d {} -D {} | awk '{{print $2}}'".format(kernelversion, kerneldir, module[:-3])
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE).decode()
        # Extract the list of dependencies from the output
        depends = []
        for mod in output.split():
            if os.path.basename(mod) != module:
                depends.append(os.path.basename(mod))
        # print(f"{os.path.basename(module)} : {', '.join(depends)}")
        return depends
    except subprocess.CalledProcessError as e:
        print(f"Error getting dependencies for module {module}: {e.output}")
        return []

def get_module_all_dependencies(record, module):
    deps_list = get_module_dependencies(module)
    for item in deps_list:
        if item not in record:
            record.append(item)
            get_module_all_dependencies(record, item)

def get_all_modules(directory):
    module_list = []
    try:
        cmd = "find {} -name '*.ko' -type f".format(directory)
        print(cmd)
        output = subprocess.check_output(cmd, shell=True, stderr=subprocess.PIPE).decode()

        for entry in output.split():
            module_list.append(os.path.basename(entry))
        return module_list
    except subprocess.CalledProcessError as e:
        print(f"Error getting module for {directory}: {e.output}")
        return []

def generate_dependency_trees(target_modules, module_list, denped_output):
    max_height = 0

    dependency_dict = {}
    for module in module_list:
        dependency_dict[module] = get_module_dependencies(module)

    # Generate a file with module names for all trees of the same height
    height_module_dict = {}
    for module in target_modules:
        height = get_tree_height(module, dependency_dict)
        if height not in height_module_dict:
            height_module_dict[height] = []
        height_module_dict[height].append(module)
        if height > max_height:
            max_height = height

    for height in range(1, max_height + 1):
        modules = height_module_dict[height]
        print(f"Height {height}:")
        for item in modules:
            print(f"\t{item}")
    if denped_output != None:
        with open(denped_output, 'w') as f:
             for height in range(1, max_height + 1):
                modules = height_module_dict[height]
                f.write(f"Height {height}:\r\n")
                for item in modules:
                    if not is_buildin(item):
                        f.write(f"\t{item}\r\n")
                    else:
                        print('{} is buildin'.format(item))

def get_tree_height(module, dependency_dict):
    if module not in dependency_dict:
        return 1
    max_height = 0
    for dependency in dependency_dict[module]:
        height = get_tree_height(dependency, dependency_dict)
        max_height = max(max_height, height)
    return max_height + 1

if __name__ == "__main__":
    # modprobe -S 5.15.78 -d ../staging/ -D imx319_mipi
    module_list = None

    earlymodule_list = []
    if earlymods is not None:
        for item in earlymods.split():
            deps = []
            earlymodule_list.append(item.strip() + '.ko')
            get_module_all_dependencies(deps, item.strip() + '.ko')
            earlymodule_list.extend(deps)

    print(earlymodule_list)

    if inputfile is not None:
        module_list = []
        with open(inputfile, 'r') as f:
            for line in f:
                module_list.append(os.path.basename(line.strip()))
    elif moddir is not None:
        module_list = get_all_modules(moddir)

    if builtinfile != None:
        with open(builtinfile, 'r') as f:
            for line in f:
                builtin_list.append(os.path.basename(line.strip()))

    if os.path.exists(outfile):
        os.remove(outfile)
    if os.path.exists(earlyfile):
        os.remove(earlyfile)

    generate_dependency_trees(module_list, module_list, outfile)
    if len(earlymodule_list) > 0:
        generate_dependency_trees(earlymodule_list, module_list, earlyfile)
