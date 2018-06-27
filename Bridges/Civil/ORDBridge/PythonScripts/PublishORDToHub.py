# --------------------------------------------------------------------------------------
#      $Source: ORDBridge/PythonScripts/PublishORDToHub.py $
#   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#--------------------------------------------------------------------------------------

from Tkinter import *
import tkFileDialog
import ttk
import win32api
import win32con
import win32event
import win32file
import win32process
import win32security


class MainWindow:
    __server_env_values = ['QA', 'PROD']


    def __init__(self):
        self.__root = Tk()
        self.__root.title('Publish ORD To iModel Hub')
        self.__initialize_controls()


    def show_dialog(self):
        self.__root.mainloop()


    def __initialize_controls(self):
        self.__root.columnconfigure(0, weight=1)
        self.__root.rowconfigure(0, weight=1)
        self.__root.rowconfigure(1, weight=1)
        self.__root.rowconfigure(2, weight=0)

        self.__initialize_fwk_controls()
        self.__initialize_dms_controls()
        self.__initialize_misc_controls()


    def __initialize_fwk_controls(self):
        self.__fwk_label_frame = LabelFrame(self.__root, text='Main Options')
        self.__fwk_label_frame.grid(row=0, column=0, padx=5, pady=5, sticky=NSEW)

        self.__fwk_exe_label = Label(self.__fwk_label_frame, text='iModelBridgeFwk.exe Path:')
        self.__fwk_exe_label.grid(row=0, column=0, padx=5, pady=5, sticky=W)
        self.__fwk_exe_entry_var = StringVar()
        self.__fwk_exe_entry = Entry(self.__fwk_label_frame, textvariable=self.__fwk_exe_entry_var)
        self.__fwk_exe_entry.grid(row=0, column=1, padx=5, pady=5, sticky=NSEW)
        self.__fwk_exe_browse_button = Button(self.__fwk_label_frame, text='...',
                                              command=self.__fwk_exe_browse_callback)
        self.__fwk_exe_browse_button.grid(row=0, column=2, padx=5, pady=5)

        self.__bridge_lib_label = Label(self.__fwk_label_frame, text='Bridge Library Path:')
        self.__bridge_lib_label.grid(row=1, column=0, padx=5, pady=5, sticky=W)
        self.__bridge_lib_entry_var = StringVar()
        self.__bridge_lib_entry = Entry(self.__fwk_label_frame, textvariable=self.__bridge_lib_entry_var)
        self.__bridge_lib_entry.grid(row=1, column=1, padx=5, pady=5, sticky=NSEW)
        self.__bridge_lib_browse_button = Button(self.__fwk_label_frame, text='...',
                                                 command=self.__bridge_lib_browse_callback)
        self.__bridge_lib_browse_button.grid(row=1, column=2, padx=5, pady=5)

        self.__staging_dir_label = Label(self.__fwk_label_frame, text='Staging Directory:')
        self.__staging_dir_label.grid(row=2, column=0, padx=5, pady=5, sticky=W)
        self.__staging_dir_entry_var = StringVar()
        self.__staging_dir_entry = Entry(self.__fwk_label_frame, textvariable=self.__staging_dir_entry_var)
        self.__staging_dir_entry.grid(row=2, column=1, padx=5, pady=5, sticky=NSEW)
        self.__staging_dir_browse_button = Button(self.__fwk_label_frame, text='...',
                                                  command=self.__staging_dir_browse_callback)
        self.__staging_dir_browse_button.grid(row=2, column=2, padx=5, pady=5)

        self.__input_file_label = Label(self.__fwk_label_frame, text='Input File:')
        self.__input_file_label.grid(row=3, column=0, padx=5, pady=5, sticky=W)
        self.__input_file_entry_var = StringVar()
        self.__input_file_entry = Entry(self.__fwk_label_frame, textvariable=self.__input_file_entry_var)
        self.__input_file_entry.grid(row=3, column=1, padx=5, pady=5, sticky=NSEW)
        self.__input_file_browse_button = Button(self.__fwk_label_frame, text='...',
                                                 command=self.__input_file_browse_callback)
        self.__input_file_browse_button.grid(row=3, column=2, padx=5, pady=5)

        self.__server_project_label = Label(self.__fwk_label_frame, text='CONNECT Project:')
        self.__server_project_label.grid(row=4, column=0, padx=5, pady=5, sticky=W)
        self.__server_project_entry_var = StringVar()
        self.__server_project_entry = Entry(self.__fwk_label_frame, textvariable=self.__server_project_entry_var)
        self.__server_project_entry.grid(row=4, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_repo_label = Label(self.__fwk_label_frame, text='CONNECT iModel Name:')
        self.__server_repo_label.grid(row=5, column=0, padx=5, pady=5, sticky=W)
        self.__server_repo_entry_var = StringVar()
        self.__server_repo_entry = Entry(self.__fwk_label_frame, textvariable=self.__server_repo_entry_var)
        self.__server_repo_entry.grid(row=5, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_user_label = Label(self.__fwk_label_frame, text='User Name:')
        self.__server_user_label.grid(row=6, column=0, padx=5, pady=5, sticky=W)
        self.__server_user_entry_var = StringVar()
        self.__server_user_entry = Entry(self.__fwk_label_frame, textvariable=self.__server_user_entry_var)
        self.__server_user_entry.grid(row=6, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_password_label = Label(self.__fwk_label_frame, text='Password:')
        self.__server_password_label.grid(row=7, column=0, padx=5, pady=5, sticky=W)
        self.__server_password_entry_var = StringVar()
        self.__server_password_entry = Entry(self.__fwk_label_frame, textvariable=self.__server_password_entry_var,
                                             show='*')
        self.__server_password_entry.grid(row=7, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_env_label = Label(self.__fwk_label_frame, text='Server Environment:')
        self.__server_env_label.grid(row=8, column=0, padx=5, pady=5, sticky=W)
        self.__server_env_combo_var = StringVar()
        self.__server_env_combo = ttk.Combobox(self.__fwk_label_frame, textvariable=self.__server_env_combo_var,
                                               state='readonly', values=MainWindow.__server_env_values)
        self.__server_env_combo.grid(row=8, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__server_env_combo_var.set(MainWindow.__server_env_values[0])

        self.__create_repo_checkbutton_var = IntVar()
        self.__create_repo_checkbutton = Checkbutton(self.__fwk_label_frame, text='Create Repository (if necessary)?',
                                                     variable=self.__create_repo_checkbutton_var)
        self.__create_repo_checkbutton.grid(row=9, column=0, columnspan=3, padx=5, pady=5, sticky=W)
        self.__create_repo_checkbutton_var.set(1)

        self.__skip_assignment_check_checkbutton_var = IntVar()
        self.__skip_assignment_check_checkbutton = Checkbutton(self.__fwk_label_frame, text='Skip Assignment Check?',
                                                               variable=self.__skip_assignment_check_checkbutton_var)
        self.__skip_assignment_check_checkbutton.grid(row=10, column=0, columnspan=3, padx=5, pady=5, sticky=W)
        self.__skip_assignment_check_checkbutton_var.set(1)

        self.__fwk_label_frame.columnconfigure(1, weight=1)


    def __initialize_dms_controls(self):
        self.__dms_label_frame = LabelFrame(self.__root, text='Managed Workspace Options')
        self.__dms_label_frame.grid(row=1, column=0, padx=5, pady=5, sticky=NSEW)


    def __initialize_misc_controls(self):
        self.__bottom_button_frame = Frame(self.__root)
        self.__bottom_button_frame.grid(row=2, column=0, padx=5, pady=5, sticky=NSEW)

        self.__publish_button = Button(self.__bottom_button_frame, text='Publish',
                                       command=self.__publish_button_callback)
        self.__publish_button.pack(side=RIGHT)


    def __fwk_exe_browse_callback(self):
        path = tkFileDialog.askopenfilename(initialdir='.', title='Find iModelBridgeFwk.exe...',
                                     filetypes=[('Executable Files', '*.exe')])
        if path:
            self.__fwk_exe_entry_var.set(path.replace('/', '\\'))


    def __bridge_lib_browse_callback(self):
        path = tkFileDialog.askopenfilename(initialdir='.', title='Find bridge library...',
                                            filetypes=[('Library Files', '*.dll')])
        if path:
            self.__bridge_lib_entry_var.set(path.replace('/', '\\'))


    def __staging_dir_browse_callback(self):
        path = tkFileDialog.askdirectory(initialdir='.', title='Select staging directory...')
        if path:
            self.__staging_dir_entry_var.set(path.replace('/', '\\'))


    def __input_file_browse_callback(self):
        path = tkFileDialog.askopenfilename(initialdir='.', title='Find input file...',
                                            filetypes=[('OpenRoads Designer Files', '*.dgn')])
        if (path):
            self.__input_file_entry_var.set(path.replace('/', '\\'))


    def __publish_button_callback(self):
        command_string = self.__prepare_command_string()
        self.__execute_command_string(command_string)


    def __prepare_command_string(self):
        command_string = '{fwk_exe} --server-project="{server_proj}" --server-repository="{server_repo}" ' \
               '--server-environment={server_env} --server-user="{server_user}" ' \
               '--server-password="{server_password}" --fwk-input="{fwk_input}" --fwk-staging-dir="{staging_dir}" ' \
               '--fwk-bridge-library="{bridge_lib}"'.format(fwk_exe=self.__fwk_exe_entry_var.get(),
                                                            server_proj=self.__server_project_entry_var.get(),
                                                            server_repo=self.__server_repo_entry_var.get(),
                                                            server_env=self.__server_env_combo_var.get(),
                                                            server_user=self.__server_user_entry_var.get(),
                                                            server_password=self.__server_password_entry_var.get(),
                                                            fwk_input=self.__input_file_entry_var.get(),
                                                            staging_dir=self.__staging_dir_entry_var.get(),
                                                            bridge_lib=self.__bridge_lib_entry_var.get())
        if self.__create_repo_checkbutton_var.get() == 1:
            command_string += ' --fwk-create-repository-if-necessary'
        if self.__skip_assignment_check_checkbutton_var.get() == 1:
            command_string += ' --fwk-skip-assignment-check'
        return command_string


    def __execute_command_string(self, command_string):
        security_attributes = win32security.SECURITY_ATTRIBUTES()
        security_attributes.bInheritHandle = True
        startup_info = win32process.STARTUPINFO()
        startup_info.dwFlags = win32process.STARTF_USESTDHANDLES
        #stdout_file_handle = win32file.CreateFile(abs_stdout_file, win32file.GENERIC_WRITE,
        #                                          win32file.FILE_SHARE_READ | win32file.FILE_SHARE_WRITE,
        #                                          security_attributes, win32file.CREATE_ALWAYS,
        #                                          win32file.FILE_FLAG_SEQUENTIAL_SCAN, None)
        #startup_info.hStdOutput = stdout_file_handle
        startup_info.hStdOutput = win32api.GetStdHandle(win32api.STD_OUTPUT_HANDLE)
        startup_info.hStdError = win32api.GetStdHandle(win32api.STD_ERROR_HANDLE)
        startup_info.hStdInput = win32api.GetStdHandle(win32api.STD_INPUT_HANDLE)
        process_info = win32process.CreateProcess(None, command_string, None, None, True, win32con.CREATE_NO_WINDOW,
                                                  None, None, startup_info)
        process_handle = process_info[0]
        return_code = win32event.WaitForSingleObject(process_handle, win32event.INFINITE)
        if return_code == win32event.WAIT_OBJECT_0:
            publisher_error_code = win32process.GetExitCodeProcess(process_handle)
        else:
            publisher_error_code = 0x8000
        #win32file.CloseHandle(stdout_file_handle)
        return publisher_error_code


if __name__ == '__main__':
    main_window = MainWindow()
    main_window.show_dialog()
