import argparse

module_name = ""
target_names = []
object_names = []


def ParseArgs():
    global module_name, target_names, object_names

    parser = argparse.ArgumentParser(
        description='Express Server Project Build Copilot for Makefile')
    # Positional arguments
    parser.add_argument('name', help='make module name.')

    # Optional arguments
    parser.add_argument(
        '-t', '--target', help='module targets name(use comma to split)')
    parser.add_argument(
        '-o', '--object', help='module object generator name(use comma to split)')

    args = parser.parse_args()

    module_name = args.name
    if args.target:
        target_names = args.target.split(',')
    if args.object:
        object_names = args.object.split(',')


def GenerateMakeModule():
    global module_name
    global target_names
    global object_names

    text = "{f_header}\n{f_targets}\n{f_all_targets}\n{f_phony}\n{f_src_dir}\n\n"
    header = f'\n\n# ==========================================================\n'\
        f'#						{module_name}\n'\
        f'# All targets about {module_name}\n'\
        f'# Note:\n'\
        f'# =========================================================='

    targets = ''
    all_targets = '{:<25}+= '.format("ALL_TARGETS")
    phony = '{:<25}+= '.format(".PHONY")
    module_src_dir = '{:<25}:= '.format(module_name+"_SRC_DIR")

    for target_name in target_names:
        target = module_name+'_'+target_name + '_TARGET'
        targets += '{:<25}:=\n'.format(target)
        all_targets += f'$({target}) '
        phony += f'$({target}) '

    for target_name in target_names:
        target = module_name + '_' + target_name+'_TARGET'
        src_dir = module_name + '_SRC_DIR'
        dep_files = module_name + '_' + target_name + '_DEP_FILES'
        src_files = module_name + '_' + target_name+'_SRC_FILES'
        srcs =module_name + '_' +  target_name+'_SRCS'
        objs = module_name + '_' + target_name+'_OBJS'

        target_part = '# About target : {}\n\n'\
            '{:<25}:= \n\n'\
            '{:<25}:= \n\n'\
            '{:<25}:= $(addprefix $({}),$({})) \\\n{:<27}$({})\n\n'\
            '{:<25}:= $({}:%=$(BUILD_DIR)/%.o)\n\n'\
            '$({}): $({})\n'\
            '\tmkdir -p $(BIN_DIR)\n'\
            '\t$(CXX) $^ -o $(BIN_DIR)/$@\n'\
            '\t@echo "$(FONT_WHITE)Generated program $(BIN_DIR)/$@$(FONT_RESET)"\n'\
            .format(target,
                    dep_files,
                    src_files,
                    srcs, src_dir, src_files, " ",dep_files,
                    objs, srcs,
                    target, objs)
        text += target_part + '\n'

    for object_name in object_names:
        src_dir = module_name + '_SRC_DIR'
        dep_files = module_name + '_'+object_name + '_DEP_FILES'
        src_files = module_name + '_' + object_name+'_SRC_FILES'
        srcs = module_name + '_' + object_name+'_SRCS'
        objs = module_name + '_' + object_name+'_OBJS'

        object_part = '# About object :{} \n\n'\
            '{:<25}:= \n\n'\
            '{:<25}:= \n\n'\
            '{:<25}:= $(addprefix $({}),$({})) \ \n{}\n\n'\
            '{:<25}:= $({}:%=$(BUILD_DIR)/%.o)\n\n'\
            .format(objs,
                    dep_files,
                    src_files,
                    srcs, src_files, src_dir,dep_files,
                    objs, srcs)
        text += object_part + '\n'

    with open('Makefile', "a") as f:
        f.write(text.format(f_header=header, f_targets=targets, f_all_targets=all_targets,
                            f_phony=phony, f_src_dir=module_src_dir))


if __name__ == '__main__':
    ParseArgs()
    GenerateMakeModule()
    print('Generate make module : {}'.format(module_name))
