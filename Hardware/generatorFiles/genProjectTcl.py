import os
import optparse

createProjectStr = """
create_project {0} {1} -part xc7z010clg400-1
set_property board_part digilentinc.com:zybo-z7-10:part0:1.0 [current_project]
set_property target_language VHDL [current_project]
"""

def genProjectTcl(projectName, tclFileName):
    try:
        os.mkdir("../vivadoProjects/")
    except FileExistsError:
        pass

    tclFile = open(tclFileName, 'w')
    projectAbsPath = os.path.abspath("../vivadoProjects") + '/' + projectName
    tclFile.write(createProjectStr.format(projectName, projectAbsPath))

    with open('../tclScripts/genBDFiles.tcl', 'r') as f_in:
        for line in f_in:
            tclFile.write(line)


    # tclFile.write("source {0}\n".format(os.path.abspath('../tclScripts/genBDFiles.tcl')))
    tclFile.write("update_compile_order -fileset sources_1\n")

    with open('../confFiles/bdList.conf', 'r') as f_in:
        bdNames = list(line for line in (l.strip() for l in f_in) if line)

    for bd in bdNames:
        fullBdPath = projectAbsPath + '/' + projectName + '.srcs/sources_1/bd/' + \
            bd + '/' + bd + '.bd'
        tclFile.write("make_wrapper -files [get_files " + fullBdPath + '] -top\n')
        fullWrapperPath = projectAbsPath  + '/' + projectName + '.srcs/sources_1/bd/' + \
            bd + '/hdl/' + bd + '_wrapper.vhd'
        tclFile.write("add_files -norecurse " + fullWrapperPath + '\n' )
    

    ## Add files
    files = os.listdir("../autoHdlFiles/")
    folderAbsPath = os.path.abspath('../autoHdlFiles') + '/'
    files = list((folderAbsPath + f) for f in files)
    tclFile.write("add_files -norecurse -scan_for_includes {" + " ".join(files) + "}\n")
    files = os.listdir("../hdlFiles/")
    folderAbsPath = os.path.abspath('../hdlFiles/') + '/'
    files = list((folderAbsPath + f) for f in files)
    tclFile.write("add_files -norecurse -scan_for_includes {" + " ".join(files) + "}\n")

    tclFile.write("update_compile_order -fileset sources_1\n")

    tclFile.write("""
set_property SOURCE_SET sources_1 [get_filesets sim_1]
add_files -fileset sim_1 -norecurse -scan_for_includes {""")
    files = os.listdir("../simHdlFiles/")
    folderAbsPath = os.path.abspath('../simHdlFiles/') + '/'
    files = list((folderAbsPath + f) for f in files)
    tclFile.write(" ".join(files) + "}\n")
    tclFile.write("update_compile_order -fileset sim_1\n")

    tclFile.write("add_files -fileset constrs_1 -norecurse " +os.path.abspath('../constraintsFiles/Connections.xdc') + '\n')

    tclFile.close()

if __name__ == "__main__":
    opt = optparse.OptionParser()
    opt.add_option('--name', '-n', default = "project_LUDH", dest="filename",action="store", type="string", help = "Project Name")
    (options, arguments) = opt.parse_args()
    projectName = options.filename

    presentProjects = set(os.listdir('../vivadoProjects/'))

    # print(presentProjects)
    if projectName in presentProjects:
        print("Project with the same name exists in the 'vivadoProjects' directory")
        exit()

    genProjectTcl("project_LUDH", '../tclScripts/projectGen.tcl')

