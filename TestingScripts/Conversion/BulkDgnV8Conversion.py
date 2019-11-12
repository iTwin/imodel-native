#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import os
import sys
import subprocess
import csv
import datetime
import argparse
from multiprocessing import Process, Lock, Array, cpu_count, Value
from collections import defaultdict
import xml.etree.ElementTree as ET
import re
import time
import signal
# import psutil
import shutil

reload(sys)  
sys.setdefaultencoding('Cp1252')

wait_for_process= 2*60*60
no_of_threads = cpu_count() - 2

if no_of_threads>4:
    no_of_threads -= 2

#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def isExe(path):
    '''
        path: path of an exe file
        return: return True if path is exe otherwise False

    '''
    if os.path.isfile(path):
        extsn = os.path.splitext(path)[1]
        if extsn == '.exe':
            return True
    return False
#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------


def get_file_text(file_name, output_dir):
    rootName = os.path.splitext(file_name)[0]
    
    for ext in ['.ibim-issues', '.bim-issues']:
        issueFilePath = os.path.join(output_dir, rootName + ext)
        if os.path.isfile(issueFilePath):
            with open(issueFilePath, 'r') as issueFile:
                return issueFile.read()
    
    return None

#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def run_cmd(cmd, status_call):
    
    status = subprocess.call(cmd)
    print status
    status_call.value = status


#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def get_bim_size(file_name, output_dir):
    
    size = -1
    pure_file_name = os.path.splitext(file_name)[0]
    for ext in ['.ibim', '.bim']:
        file_path = os.path.join(output_dir, pure_file_name + ext)
        if os.path.isfile(file_path):
            size = os.stat(file_path).st_size/1024.0/1024.0        
    return size

#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def parallelism_function(cmd, file_name, output_dir, report_lock, ls_files_counter):
    '''
        This function is used for multiprocessing
        cmd : cmd to be executed 
        output_dir : output directory path
        report_lock : lock to access the csv file so that at a time only one process access it
        ls_files_counter : list for the summary of files

    '''

    
    line = []                                   #list to write in csv file in append mode
    line.append(datetime.datetime.now())        #Af first index writing time at now
    line.append(file_name)                      #At second index writing file name

    result = 'PASS'                             #result which will be saved at 3rd index of list
    
    #Log_file
    log_file = "log_file_1.txt"
    output_file = os.path.join(output_dir, log_file)
    
    report_lock.acquire()
    check_file = open(output_file, "a+")
    check_file.write(file_name + ",   " + cmd + "\n")
    check_file.close()
    report_lock.release()
    #Log_file

    return_status = Value('i', -1)
    
    sub_p = Process(target=run_cmd, args=(cmd,return_status))
    sub_p.start()
    sub_p.join(wait_for_process)
    if return_status.value == -1:
        print 'killing process', file_name
        os.kill(sub_p.pid, signal.SIGTERM)
        line.append("FAIL")
        line.append("CRASH")
        line.append(get_bim_size(file_name, output_dir))
        ls_files_counter[1] += 1
        ls_files_counter[2] += 1
        
        
    else:
        status_call = return_status.value

        notes_file = ""                             #notes which will be saved at 4th index of list
        file_text = get_file_text(file_name, output_dir)
                                                    #getting file text from file
        
        if file_text == None:
            result = "PASS"
            notes_file = "Issue file does not exist"
            ls_files_counter[1] += 1
        
        else:
            # Log_file
            log_file = "log_file_2.txt"
            output_file = os.path.join(output_dir, log_file)
            
            report_lock.acquire()
            check_file = open(output_file, "a+")
            check_file.write(file_name + " , "+str(status_call)+"\n")
            check_file.close()
            report_lock.release()
            # Log_file

            if status_call == 0:                        #if the command executes successfully increment in successful files
                ls_files_counter[0] += 1
            
                string_1 = re.search( "Failed to deserialize v8 ECSchema", file_text)        
                string_2 = re.search("Failed to import ECSchema", file_text)
                string_3 = re.search("Failed to transform the v8 ECSchemas to a BIS based ECSchema", file_text)
                                    #if there is error is schema import than it is not passed
                if string_1 or string_2 or string_3:
                    result = "FAIL"                     
                    notes_file = "SchemaImportFailure"
                    ls_files_counter[1] += 1                   #increment  in failed files
                    ls_files_counter[0] -= 1                   #decrement in successful files


                                    #if the command executed crashes
            elif status_call < 0 or status_call == 255:
                result = "FAIL"
                notes_file = "CRASH"
                ls_files_counter[1] += 1
                ls_files_counter[2] += 1
                
            elif status_call != 0:                      #if the command fails to be run
                ls_files_counter[1] += 1
                result = "FAIL"


                index = -1                              #To find the error from issue file
                for j in range(1, len(file_text)+1):    #range is started from 1 to len+1, because i am searching it from backward, and searching last occurance of 'Error' in file

                    if j+4 <= len(file_text) and file_text[-j] == "r" and file_text[-j-1] == "o" and file_text[-j-2] == "r" and file_text[-j-3] == "r" and file_text[-j-4] == "E":
                                                                                                                                        #A bit messy but efficient here for finding last occurance of 'Error'
                        index = j-2
                        break

                    elif j+4 <= len(file_text) and file_text[-j] == "l" and file_text[-j-1] == "a" and file_text[-j-2] == "t" and file_text[-j-3] == "a" and file_text[-j-4] == "F":
                                                                                                                                        #For finding last occurance of 'Fatal'
                        index = j-2
                        break
                
                if index == -1:
                    notes_file = "No occurences of Error in file"
                else:
                    index = len(file_text) - index
                    for j in range(index, len(file_text)):
                        notes_file += file_text[j]


        line.append(result)
        line.append(notes_file)
        line.append(get_bim_size(file_name, output_dir))

    ls = []
    ls.append(line)

    output_file_name = "Output_report.csv"
    output_file = os.path.join(output_dir, output_file_name)

    report_lock.acquire()

    with open(output_file, 'ab') as csvfile:    #writing that list in csv file as a row line
        csv_writer = csv.writer(csvfile)
        csv_writer.writerows(ls)

    report_lock.release()


    return 



#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def count_process(p_list):
    
    if type(p_list) is not list:
        if p_list.is_alive():
            return 1
        else:
            return 0
    counter = 0 
    for p in p_list:
        counter += count_process(p)
    return counter




#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def limit_process(p_list, max_p_count):
    
    while True:
        p_count = count_process(p_list)
        if p_count < max_p_count:
            break

#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def recursive_converter(dgn_exe_path, dgn_args_dict, folder_path, output_path, report_lock, ls_files, custom_check, ls_files_counter):
    '''
        A Recursive code for visiting all the folder in path, and acess all the dgn files in it.

        dgn_exe_path     : path of DgnConverter.exe
        dgn_args_dict    : dictionary of the arguments dgn converter.exe
                            (key->argument name, value-> argument value) 
        folder_path      : path of this current folder
        output_path      : path where file will be saved after conversion
        report_lock      : Lock for the csv file so that at a time it will be accessed by only one process
        ls_files         : list of files to be converted
        custom_check     : a boolean variable to know that custom argument is used or not
        ls_files_counter : an array for the summary of files

    '''

    exe_type = os.path.split(dgn_exe_file)[-1]           
    curr_files = os.listdir(folder_path)        #list of all files in current folder path
    no_of_files = len(curr_files)
    p_list = []

    for i in range(no_of_files):

        file_path = os.path.join(folder_path, curr_files[i])                  #path of files in current directory
        
        if os.path.isdir(file_path) is True:                #checking the file is directory or not, if directory then call same fun on it too
           
            p_list.append(recursive_converter(dgn_exe_path, dgn_args_dict, file_path, output_path, report_lock, ls_files, custom_check, ls_files_counter))
        elif os.path.splitext(file_path)[1] == ".dgn":          #checking file is dgn file or not

            if ls_files is not None and custom_check is True:   #checking that custom argument is used or not
                curr_file = curr_files[i]
                found = False
                for j in range(len(ls_files)):
                    if curr_file == ls_files[j]:
                        found = True
                        ls_files.remove(curr_file)
                        break
                    
            if (custom_check is True and found is True) or custom_check is False:       #Whether this file in csv file is to be converted or not

                
                if(exe_type == 'IDgnToIDgnDb.exe'):

                    print '\n\n***** Importer Conversion *****\n\n'
                    
                    if (not IbimFlag):
                        print 'Please provide a yes/no flag for ibim creation. Exiting'
                        exit(1)
                        
                    file_name = os.path.split(file_path)[-1]
                    file_without_extension = file_name.split('.')[0]
                    file_with_extension = file_without_extension + '.idgndb'
                    file_with_ext_path = os.path.join(output_dir , file_with_extension)
                    compressed_file_name = file_without_extension + '.imodel'

                    #If we want to create the ibim files along with the imodels
                    if (IbimFlag.lower()=='yes'):  
                        #Creating a folder to place the compressed imodels. Recreating it, if it already exists
                        folder_for_imodels = os.path.join(output_dir , 'CompressedImodels')
                        if(os.path.exists(folder_for_imodels)):
                            print 'Folder already exists. Deleting it'
                            shutil.rmtree(folder_for_imodels)
                        
                        os.mkdir(folder_for_imodels)
                    
                        compressed_file_path = os.path.join(folder_for_imodels , compressed_file_name)
                        input_link = "--input " + "\"" + file_path + "\""
                        output_link = "--output " + "\"" + file_with_ext_path + "\""

                        cmd = "\"" + dgn_exe_path + "\"  " + input_link +"  "+ output_link + " -z " + "\""+ compressed_file_path + "\""
                        print cmd

                    #If we don't want to create the ibim files along with the imodels
                    elif (IbimFlag.lower()=='no'):
                        
                        input_link = "--input " + "\"" + file_path + "\""
                        imodel_path = os.path.join(output_dir , compressed_file_name)
                        cmd = "\"" + dgn_exe_path + "\"  " + input_link + " -z " + "\""+ imodel_path + "\""
                        print cmd

                    #If the command has some extra parameters in the Customize.xml file to add
                    for item in dgn_args_dict.iteritems():      #appending the argument in cmd if they are
                        cmd = cmd + " --" + item[0]
                        if item[1]:
                            cmd += " \"" + item[1] + "\""
                            print cmd
                                
                else:
                    #Incase of a dgnV8 Conversion
                    output_link = "--output=" + "\"" + output_dir + "\""
                    input_link = "--input=" + "\"" + file_path + "\""
                    
                    cmd = "\"" + dgn_exe_path + "\"  " + input_link +"  "+ output_link
                    
           
                    for item in dgn_args_dict.iteritems():      #appending the argument in cmd if they are
                        cmd = cmd + " --" + item[0]
                        if item[1]:
                            cmd += "=\"" + item[1] + "\""


                limit_process(p_list, no_of_threads)
                p = Process(target=parallelism_function, args=(cmd, curr_files[i], output_path, report_lock, ls_files_counter))
                p.start()
                p_list.append(p)

        #In case of a dwg file      
        elif os.path.splitext(file_path)[1] == ".dwg":          #checking file is dgn file or not
            
            if ls_files is not None and custom_check is True:   #checking that custom argument is used or not
                curr_file = curr_files[i]
                found = False
                for j in range(len(ls_files)):
                    if curr_file == ls_files[j]:
                        found = True
                        ls_files.remove(curr_file)
                        break
            if (custom_check is True and found is True) or custom_check is False:       #Whether this file in csv file is to be converted or not


                output_link = "--output=" + "\"" + output_dir + "\""

                input_link = "--input=" + "\"" + file_path + "\""
                cmd = "\"" + dgn_exe_path + "\"  " + input_link +"  "+ output_link

               
                for item in dgn_args_dict.iteritems():      #appending the argument in cmd if they are
                    cmd = cmd + " --" + item[0]
                    print '\n\n\ncmd:' + cmd
                    if item[1]:
                        cmd += "=\"" + item[1] + "\""

                limit_process(p_list, no_of_threads)
                p = Process(target=parallelism_function, args=(cmd, curr_files[i], output_path, report_lock, ls_files_counter))
                p.start()
                p_list.append(p)
                
    return p_list                                                   #returning the list of process created in this folder

#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def read_csv_file(csv_path, filters_dict):
    '''
        Read the csv file and return all filtered files names in a list

        csv_path : Path of the csv files 
        filters_dict : dictionary of filters 
                        (key->column name, value->value of column on which basis file will be filtered)

    '''
    
    ls = []
    with open(str(csv_path), 'r') as f:                         #reading the csv files
        file = f.readlines()

    column_names = file[0].split(',')                           #spliting the first line of csv for the getting column names in it
    no_of_col = len(column_names)
    column_names[no_of_col-1] = column_names[no_of_col-1].strip("\n")       #stripping "\n" from last value of the array


    files = []

    for i in range(1, len(file)):                               #reading all lines of csv except first line of column names
        line = file[i].split(',')
        len_of_line = len(line)
        line[len_of_line-1] = line[len_of_line-1].strip("\n")

        skip = False
        for j in range(1, no_of_col):                           #checking whether file in this line is to be convereted or not
            skip_value = filters_dict[column_names[j]]
            if skip_value != 0 and skip_value.upper() != line[j].upper():
                skip = True
                break

        if skip is False:
            files.append(line[0])

    return files

#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def custom_contents(xml_path):
    '''
        xml_path : Path of the xml file
        Return : return the names of all those file to be converted, exe args names, exe args values
    '''

    tree = ET.parse(xml_path)                              

    root = tree.getroot()   
    customize = root.find('Customize')                      
    data_set = customize.find('DataSetFilters')

    csv_path = data_set.find('path').text.strip(" ")

    column_filters = data_set.find('ColumnFilters')
    columns = column_filters.findall('Column')

                                     #Creating a defauldict for all those selected columns and their values in xml file
    filters_dict = defaultdict(int)
    for column in columns:
        
        if column.get("values") == "1":
            filters_dict[column.get("Name")] = column.text.strip(" ")     #column name keys, and column text as values of default dict

 

    files = read_csv_file(csv_path, filters_dict)

    dgn_args_dict=defaultdict(int)          #For DgnConverter.exe arguments

    arguments = customize.find("Arguments")

    for argument in arguments:
        if argument.get("values") == "1":
            dgn_arg_text = ""
            if argument.text:
                dgn_arg_text = argument.text.strip(" ")
            dgn_args_dict[argument.get("Name").strip(" ")] = dgn_arg_text
    
    import_config_file = customize.find("ImportConfigFile")
    icf_path = import_config_file.find("path")
    import_config_path = None
    if icf_path.get("values") == "1":
        import_config_path = icf_path.text.strip(" ")

          
    return files, dgn_args_dict, import_config_path

#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def join_process(p_list):
    '''
        Recursive Function to visit all process and check whether that process is finished, 
        if not then it will wait for process to finish.

        p_list : list of list of processes and processes
    '''
    if type(p_list) is not list:    #if a process the wait for it
        p_list.join()
        return
    for p in p_list:
        join_process(p)


#-------------------------------------------------------------------------------------------
# bsimethod                                     Akash.Dharani    07/2017
#-------------------------------------------------------------------------------------------
def move_file(config_path, dgn_path, step):
    '''
        To preserve the real ImportConfig.xml 

        config_path : Path of the new ImportConfig.xml file
        dgn_path    : Path of the dgn converter path 
        step        : 1. to preserve file, 2. move back to real place 

    '''

    tree_path_dgn = dgn_path.split("\\")
    new_path = ""
    for i in range(len(tree_path_dgn)-1):
        new_path += tree_path_dgn[i] + "\\"

    new_dir_name = "anonymous_folder"   

    import_cfg_filename = "ImportConfig.xml"
    new_path = os.path.join(new_path, 'assets')
    old_config_path = os.path.join(new_path, import_cfg_filename)
    move_to_path = os.path.join(new_path, new_dir_name)

    if step == 1:
        
        if not os.path.exists(move_to_path):
            new_dir_cmd = "mkdir " + move_to_path
            os.system(new_dir_cmd)
        backup_cfg_cmd = "move " + old_config_path + " " + move_to_path

        ret_value = os.system(backup_cfg_cmd)
        if ret_value != 0:
            print "Error1 : In moving importconfig file. Exiting."
            exit()

        move_cfg_cmd = "copy " + config_path + " " + new_path
        ret_value = os.system(move_cfg_cmd)
        if ret_value != 0:
            print "Error: In copying importconfig file from given path of ImportConfig.xml. Exiting."
            move_file(config_path, dgn_path, 2)         #put back the ImportConfig.xml to its original place
            exit()
    elif step == 2:
        if os.path.isfile(old_config_path):
            delete_cmd = "del "+ old_config_path
            ret_value = os.system(delete_cmd)
            if ret_value != 0:
                print "Warning: In deleting the importconfig file in new folder named ", new_dir_name

        #move_cfg_back = "move " + move_to_path + "\\ImportConfig.xml " + new_path
        move_cfg_back = "move " + os.path.join(move_to_path, import_cfg_filename) + " " + new_path 
        ret_value = os.system(move_cfg_back)
        if ret_value != 0:
            print "Warning: In moving old importconfig file back to its original path"

        del_dir_cmd = "rmdir "+ move_to_path
        ret_value = os.system(del_dir_cmd)
        if ret_value != 0:
            print "Warning: Can not delete the new folder named ", new_dir_name
    else:
        print "Error : Invalid value of step variable"

    


# #-------------------------------------------------------------------------------------------
# # bsimethod                                     Akash.Dharani    07/2017
# #-------------------------------------------------------------------------------------------
# def cleaning(process_name):
    
#     kill_process = process_name
#     for proc in psutil.process_iter():
#         if proc.name() == kill_process:
#             print proc.name()
#             children = proc.children(recursive=True)  
#             for _child in children:
#                 _child.kill()
#             proc.kill()
    


#---Entry point of the script ---#    
if __name__ == '__main__':

    '''
        Input:  (required) --converter : DgnCoverter.exe,
                (required) --input     : Path of directory of files to covert,
                (required) --output    : Path of directory where coverted files will be saved
                (optional) --custom    : Path of the xml file 
        
    '''

    before_time = datetime.datetime.now()

    parser = argparse.ArgumentParser()
    parser.add_argument("--converter", help="Path of Converter", required=True)
    parser.add_argument("--input", help="Path of the directory where dgn files reside for converting", required=True)
    parser.add_argument("--output", help="Path of the directory where converted files will be saved. \
                                    Note : If this directory does not exist then it will be created.", required=True)
    parser.add_argument("--custom", help="Path of xml file to convert files from csv file")
    parser.add_argument("--ibimFlag", help="Incase of importer conversion, if this flag is yes, ibims are created otherwise no ibims would be created")

    args = parser.parse_args()

    #Getting the values of arguements
    dgn_exe_file = args.converter
    input_dir = args.input
    output_dir = args.output
    custom_file = args.custom
    IbimFlag = args.ibimFlag


    if not isExe(dgn_exe_file):
        print "Error : Invalid Coverter Exe path. Exiting."
        exit()


    if not os.path.isdir(input_dir):
        print "Error : Invalid path for input Directory. Exiting."
        exit()


    if not os.path.isdir(output_dir):
        print "Warning : Output Directory does not exist, so creating it."
        os.makedirs(output_dir)

    print "All given paths are good. Starting conversion..."
    ls_files = None


    dgn_args_dict=defaultdict(int)
    custom_check = False
    if custom_file is not None:     #if custom argument is used
        if os.path.isfile(custom_file):     #checking the custom path
            ls_files, dgn_args_dict, import_cfg_path = custom_contents(custom_file)         #reading the xml file
            if import_cfg_path is not None:
                move_file(import_cfg_path, dgn_exe_file, 1)

            custom_check = True
        else:
            print "Error : Invalid Custom xml file path. Exiting."
            exit()

    ls = []
    ls.append(['Date', 'Name', 'Result', 'Notes', 'bim_size'])      #columns in output csv file
    output_file = output_dir + "\\" + "Output_report.csv"
    with open(output_file, 'wb') as csvfile:
        csv_writer = csv.writer(csvfile)
        csv_writer.writerows(ls)

    report_lock = Lock()  #Lock for accessing output csv file by one process at a time

    
    ls_files_counter = Array('i', [0,0,0])       #Shared array of multiprocessing for the summary of files
    process_list = recursive_converter(dgn_exe_file, dgn_args_dict, input_dir, output_dir, report_lock, ls_files, custom_check, ls_files_counter)

    join_process(process_list)                  #wait for all process to finish
    after_time = datetime.datetime.now()
    diff_time = after_time-before_time

    if custom_check is True and import_cfg_path is not None:
        move_file(import_cfg_path, dgn_exe_file, 2)

    Parts = os.path.split(dgn_exe_file)
    ExeName = Parts[-1]
    # if (ExeName == 'DwgImporter.exe'):
    #     cleaning("DwgImporter.exe")
    # elif(ExeName == 'DgnV8Converter.exe'):
    #     cleaning("DgnV8Converter.exe")
    # elif(ExeName == 'IDgnToIDgnDb.exe'):
    #     cleaning("IDgnToIDgnDb.exe")
    
    
    print "\n\n\n\n\n\n"
    print "**********************************************************************"
    print "\n\n\n"
    print "\t Total Files : ", ls_files_counter[0]+ls_files_counter[1]
    #print "\t Successful  : ", ls_files_counter[0]
    #print "\t Failed      : ", ls_files_counter[1]
    #print "\t Crashed     : ", ls_files_counter[2]
    print "\t Time        : ", diff_time
    print "\n\n\n"
    print "***********************************************************************"
    
