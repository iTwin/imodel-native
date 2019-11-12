"""
Runs the given publisher executable on all design files found in the given search paths.
"""

# ---------------------------------------------------------------------------------------------
#   Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#   See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------

import argparse
import os.path
import sys
import win32api
import win32con
import win32event
import win32file
import win32process
import win32security
import xlsxwriter


class ReturnCodeInfo:
    """
    Manages the return code information.
    """
    __error_map = {
        0x0000: ('Pass', 'INFO: The conversion completed successfully.'),
        0x0001: ('Affinity Check Failure', 'INFO: This file has an Affinity Level of "None."'),
        0x0002: ('Affinity Check Failure', 'INFO: This file has an Affinity Level of "Low."'),
        0x0003: ('Affinity Check Failure', 'INFO: This file has an Affinity Level of "Medium."'),
        0x0004: ('Affinity Check Failure', 'INFO: This file has an Affinity Level of "High."'),
        0x1001: ('Failure', 'ERROR: Failed to parse command line arguments.'),
        0x1002: ('Failure', 'ERROR: Failed to initialize the iModel Bridge.'),
        0x8000: ('Failure', 'ERROR: General failure.')
    }


    def __init__(self, return_code):
        self.__return_code = return_code


    def get_return_code(self):
        return self.__return_code


    def get_error_string(self):
        if self.__return_code in self.__error_map:
            return self.__error_map[self.__return_code][1]
        else:
            return self.__error_map[0x8000][1]


    def get_result_status(self):
        if self.__return_code in self.__error_map:
            return self.__error_map[self.__return_code][0]
        else:
            return self.__error_map[0x8000][0]


class ReportWriter:
    """
    Manages the writing of the report file.
    """
    def __init__(self, abs_report_file, num_results):
        self.__abs_report_file = abs_report_file
        self.__num_results = num_results
        self.__workbook = xlsxwriter.Workbook(abs_report_file)
        self.__status_worksheet = self.__workbook.add_worksheet('Status')
        self.__status_worksheet.add_table('A1:E{count}'.format(count=num_results+1),
                                          {
                                              'style': 'Table Style Medium 9',
                                              'columns': [
                                                  {'header': 'Design File'},
                                                  {'header': 'BIM File'},
                                                  {'header': 'Log File'},
                                                  {'header': 'Status'},
                                                  {'header': 'Return Value String'}
                                              ]
                                          })
        format_pass = self.__workbook.add_format({'bg_color': '#00B050'})
        format_affinity_check_failure = self.__workbook.add_format({'bg_color': '#FFC000'})
        format_failure = self.__workbook.add_format({'bg_color': '#FF0000'})
        self.__status_worksheet.conditional_format('A2:E{count}'.format(count=num_results+1),
                                                   {
                                                       'type': 'formula',
                                                       'criteria': '=$D2="Pass"',
                                                       'format': format_pass
                                                   })
        self.__status_worksheet.conditional_format('A2:E{count}'.format(count=num_results+1),
                                                   {
                                                       'type': 'formula',
                                                       'criteria': '=$D2="Affinity Check Failure"',
                                                       'format': format_affinity_check_failure
                                                   })
        self.__status_worksheet.conditional_format('A2:E{count}'.format(count=num_results+1),
                                                   {
                                                       'type': 'formula',
                                                       'criteria': '=$D2="Failure"',
                                                       'format': format_failure
                                                   })



    def __del__(self):
        self.close()


    def close(self):
        if self.__workbook is not None:
            self.__workbook.close()
            self.__workbook = None


    def write_result_to_report(self, result_num, abs_design_file, abs_output_file, abs_log_file, return_code_info):
        row = result_num+1
        self.__status_worksheet.write(row, 0, os.path.basename(abs_design_file))
        self.__status_worksheet.write_comment(row, 0, abs_design_file)
        self.__status_worksheet.write(row, 1, os.path.basename(abs_output_file))
        self.__status_worksheet.write_comment(row, 1, abs_output_file)
        self.__status_worksheet.write_url(row, 2, abs_log_file, string=os.path.basename(abs_log_file))
        self.__status_worksheet.write_comment(row, 2, abs_log_file)
        self.__status_worksheet.write(row, 3, return_code_info.get_result_status())
        self.__status_worksheet.write(row, 4, return_code_info.get_error_string())


def write_message(message):
    """
    Writes a message out to standard output.

    :param message: The message to write out.
    :return: Nothing.
    """
    print(message)


def find_all_design_files(abs_input_file_search_paths, recursive):
    """
    Gathers the list of design files that live in the given search paths.

    :param abs_input_file_search_paths: The list of absolute paths to search for design files.
    :param recursive: Whether or not the input search paths should be searched recursively.

    :return: A list of the absolute paths to all of the found design files.
    """
    abs_design_files = []
    for abs_search_path in abs_input_file_search_paths:
        for dir_name, subdir_list, file_list in os.walk(abs_search_path):
            for file_name in file_list:
                if file_name.lower().endswith('.dgn'):
                    abs_design_files.append(os.path.join(dir_name, file_name))
            if not recursive:
                break
    return abs_design_files


def publish_file(publisher_file_name, publisher_path, abs_design_file, abs_output_file, abs_stdout_file):
    """
    Executes the given publisher on the given design file and outputs the given output file. Assumes the current
    working directory is the directory that contains the publisher's executable.

    :param publisher_file_name: The file name of the publisher executable.
    :param publisher_path: The path to the publisher executable.
    :param abs_design_file: The absolute path to the design file to be published.
    :param abs_output_file: The absolute path to the output file to be generated.
    :param abs_stdout_file: The absolute path to the file that stdout should be redirected to.

    :return: The error code returned from the publisher.
    """
    publisher_full_path = os.path.join(publisher_path, publisher_file_name)
    publisher_command = '{publisher} --input="{input}" --output="{output}"'.format(publisher=publisher_full_path,
                                                                                     input=abs_design_file,
                                                                                     output=abs_output_file)
    security_attributes = win32security.SECURITY_ATTRIBUTES()
    security_attributes.bInheritHandle = True
    startup_info = win32process.STARTUPINFO()
    startup_info.dwFlags = win32process.STARTF_USESTDHANDLES
    stdout_file_handle = win32file.CreateFile(abs_stdout_file, win32file.GENERIC_WRITE,
                                              win32file.FILE_SHARE_READ | win32file.FILE_SHARE_WRITE,
                                              security_attributes, win32file.CREATE_ALWAYS,
                                              win32file.FILE_FLAG_SEQUENTIAL_SCAN, None)
    startup_info.hStdOutput = stdout_file_handle
    startup_info.hStdError = win32api.GetStdHandle(win32api.STD_ERROR_HANDLE)
    startup_info.hStdInput = win32api.GetStdHandle(win32api.STD_INPUT_HANDLE)
    process_info = win32process.CreateProcess(None, publisher_command, None, None, True, win32con.CREATE_NO_WINDOW,
                                              None, publisher_path, startup_info)
    process_handle = process_info[0]
    return_code = win32event.WaitForSingleObject(process_handle, win32event.INFINITE)
    if return_code == win32event.WAIT_OBJECT_0:
        publisher_error_code = win32process.GetExitCodeProcess(process_handle)
    else:
        publisher_error_code = 0x8000
    win32file.CloseHandle(stdout_file_handle)
    return publisher_error_code


def batch_publish_ord_to_bim(abs_publisher_path, abs_input_file_search_paths, abs_output_path, recursive,
                             abs_report_path):
    """Publishes the design files found in the given list of search paths to the given output path with the publisher
    located in the given path.

    :param abs_publisher_path: The absolute path to the publisher executable.
    :param abs_input_file_search_paths: The list of absolute paths to search for design files.
    :param abs_output_path: The absolute path to the output folder.
    :param recursive: Whether or not the input search paths should be searched recursively.
    :param abs_report_path: The absolute path to the output folder.

    :return: Nothing.
    """
    abs_publisher_dir, publisher_file_name = os.path.split(abs_publisher_path)
    abs_design_files = find_all_design_files(abs_input_file_search_paths, recursive)
    design_file_count = len(abs_design_files)
    write_message('Found ' + str(design_file_count) + ' design files to process.')
    report_writer = ReportWriter(abs_report_path, design_file_count)
    current_design_file_index = 1
    for abs_design_file in abs_design_files:
        output_file_name = os.path.splitext(os.path.basename(abs_design_file))[0] + '.ibim'
        abs_output_file = os.path.join(abs_output_path, output_file_name)
        abs_stdout_file = os.path.splitext(abs_output_file)[0] + '.log'
        write_message('Processing file ' + str(current_design_file_index) + ' of ' + str(design_file_count))
        error_code = publish_file(publisher_file_name, abs_publisher_dir, abs_design_file, abs_output_file,
                                  abs_stdout_file)
        return_code_info = ReturnCodeInfo(error_code)
        report_writer.write_result_to_report(current_design_file_index-1, abs_design_file, abs_output_file,
                                             abs_stdout_file, return_code_info)
        write_message(return_code_info.get_error_string())
        current_design_file_index += 1
    report_writer.close()
    write_message('Done.')


"""Entry point."""
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Executes the ORD to BIM publisher on multiple files.")
    parser.add_argument('-i', '--input-file-search-paths', required=True, dest="input_file_search_paths",
                        help="A semi-colon delimited list of paths to search for design files in.")
    parser.add_argument('-o', '--output-path', required=True, dest="output_path",
                        help="A path to the directory where the converted files should be stored.")
    parser.add_argument('-x', '--report-path', required=True, dest="report_path",
                        help="A path to the report file (xlsx) to generate.")
    parser.add_argument('-p', '--publisher-path', required=False, dest="publisher_path",
                        help=("The path to the publisher executable. If not given, the current working directory "
                             "is assumed."))
    parser.add_argument('-r', '--recursive', action='store_true', required=False, dest="recursive",
                        help="Whether or not to search the input file search paths recursively.")
    args = parser.parse_args(sys.argv[1:])

    if args.input_file_search_paths is not None:
        input_file_search_paths = args.input_file_search_paths.split(";")
    else:
        input_file_search_paths = []

    if not input_file_search_paths:
        print('No input file search path(s) given.')
        sys.exit(2)

    abs_input_file_search_paths = []
    for input_path in input_file_search_paths:
        abs_input_path = os.path.abspath(input_path)
        if not os.path.exists(abs_input_path):
            print(abs_input_path + ' does not exist.')
            sys.exit(2)
        abs_input_file_search_paths.append(abs_input_path)

    abs_output_path = os.path.abspath(args.output_path)
    if not os.path.exists(abs_output_path):
        print(abs_output_path + ' does not exist.')
        sys.exit(2)

    abs_report_path = os.path.abspath(args.report_path)
    abs_report_dir_path = os.path.dirname(abs_report_path)
    if not os.path.exists(abs_report_dir_path):
        print(abs_report_dir_path + ' does not exist.')
        sys.exit(2)

    if args.publisher_path is not None:
        abs_publisher_path = os.path.join(os.path.abspath(args.publisher_path), "PublishORDToBIM.exe")
    else:
        abs_publisher_path = os.path.join(os.getcwd(), "PublishORDToBim.exe")

    if not os.path.exists(abs_publisher_path):
        print(abs_publisher_path + ' does not exist.')
        sys.exit(2)

    if args.recursive is not None:
        recursive = args.recursive
    else:
        recursive = False

    batch_publish_ord_to_bim(abs_publisher_path, abs_input_file_search_paths, abs_output_path, recursive,
                             abs_report_path)
