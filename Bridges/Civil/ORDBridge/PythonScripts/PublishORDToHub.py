# --------------------------------------------------------------------------------------
#      $Source: ORDBridge/PythonScripts/PublishORDToHub.py $
#   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#--------------------------------------------------------------------------------------

from Tkinter import *
import tkFileDialog
import ttk
import win32con
import win32event
import win32process
import win32security


class MainWindow:
    __server_env_values = ['QA', 'RELEASE']


    def __init__(self):
        self.__root = Tk()
        self.__root.title('Publish ORD To iModel Hub')
        self.__root.state('zoomed')
        self.__initialize_controls()


    def show_dialog(self):
        self.__root.mainloop()


    def __initialize_controls(self):
        self.__root.columnconfigure(0, weight=1)
        self.__root.rowconfigure(0, weight=0)
        self.__root.rowconfigure(1, weight=1)
        self.__root.rowconfigure(2, weight=0)

        self.__initialize_fwk_controls()
        self.__initialize_dms_controls()
        self.__initialize_misc_controls()


    def __initialize_fwk_controls(self):
        self.__fwk_label_frame = LabelFrame(self.__root, text='Main Options')
        self.__fwk_label_frame.grid(row=0, column=0, padx=5, pady=5, sticky='new')

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
        self.__create_repo_checkbutton_var.set(0)

        self.__skip_assignment_check_checkbutton_var = IntVar()
        self.__skip_assignment_check_checkbutton = Checkbutton(self.__fwk_label_frame, text='Skip Assignment Check?',
                                                               variable=self.__skip_assignment_check_checkbutton_var)
        self.__skip_assignment_check_checkbutton.grid(row=10, column=0, columnspan=3, padx=5, pady=5, sticky=W)
        self.__skip_assignment_check_checkbutton_var.set(1)

        self.__debug_cfg_checkbutton_var = IntVar()
        self.__debug_cfg_checkbutton = Checkbutton(self.__fwk_label_frame, text='Debug Cfg?',
                                                   variable=self.__debug_cfg_checkbutton_var)
        self.__debug_cfg_checkbutton.grid(row=11, column=0, columnspan=3, padx=5, pady=5, sticky=W)
        self.__debug_cfg_checkbutton_var.set(1)

        self.__fwk_label_frame.columnconfigure(1, weight=1)


    def __initialize_dms_controls(self):
        self.__dms_controls = []
        initial_state = 'disabled'

        self.__dms_label_frame = LabelFrame(self.__root, text='Managed Workspace Options')
        self.__dms_label_frame.grid(row=1, column=0, padx=5, pady=5, sticky='new')

        self.__has_managed_workspace_checkbutton_var = IntVar()
        self.__has_managed_workspace_checkbutton = Checkbutton(self.__dms_label_frame, text='Has Managed Workspace?',
                                                               variable=self.__has_managed_workspace_checkbutton_var,
                                                               command=self.__has_managed_workspace_checkbutton_callback)
        self.__has_managed_workspace_checkbutton.grid(row=0, column=0, columnspan=3, padx=5, pady=5, sticky=W)
        self.__has_managed_workspace_checkbutton_var.set(0)

        self.__dms_library_label = Label(self.__dms_label_frame, text='iModelDmsSupportB02.dll Path:',
                                         state=initial_state)
        self.__dms_library_label.grid(row=1, column=0, padx=5, pady=5, sticky=W)
        self.__dms_library_entry_var = StringVar()
        self.__dms_library_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_library_entry_var,
                                         state=initial_state)
        self.__dms_library_entry.grid(row=1, column=1, padx=5, pady=5, sticky=NSEW)
        self.__dms_library_browse_button = Button(self.__dms_label_frame, text='...',
                                              command=self.__dms_library_browse_callback, state=initial_state)
        self.__dms_library_browse_button.grid(row=1, column=2, padx=5, pady=5)
        self.__dms_controls.append(self.__dms_library_label)
        self.__dms_controls.append(self.__dms_library_entry)
        self.__dms_controls.append(self.__dms_library_browse_button)

        self.__dms_workspace_dir_label = Label(self.__dms_label_frame, text='Workspace Directory:', state=initial_state)
        self.__dms_workspace_dir_label.grid(row=2, column=0, padx=5, pady=5, sticky=W)
        self.__dms_workspace_dir_entry_var = StringVar()
        self.__dms_workspace_dir_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_workspace_dir_entry_var,
                                               state=initial_state)
        self.__dms_workspace_dir_entry.grid(row=2, column=1, padx=5, pady=5, sticky=NSEW)
        self.__dms_workspace_dir_browse_button = Button(self.__dms_label_frame, text='...',
                                                       command=self.__dms_workspace_dir_browse_callback,
                                                       state=initial_state)
        self.__dms_workspace_dir_browse_button.grid(row=2, column=2, padx=5, pady=5)
        self.__dms_controls.append(self.__dms_workspace_dir_label)
        self.__dms_controls.append(self.__dms_workspace_dir_entry)
        self.__dms_controls.append(self.__dms_workspace_dir_browse_button)

        self.__dms_user_label = Label(self.__dms_label_frame, text='DMS User Name:', state=initial_state)
        self.__dms_user_label.grid(row=3, column=0, padx=5, pady=5, sticky=W)
        self.__dms_user_entry_var = StringVar()
        self.__dms_user_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_user_entry_var,
                                      state=initial_state)
        self.__dms_user_entry.grid(row=3, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__dms_controls.append(self.__dms_user_label)
        self.__dms_controls.append(self.__dms_user_entry)

        self.__dms_password_label = Label(self.__dms_label_frame, text='DMS Password:', state=initial_state)
        self.__dms_password_label.grid(row=4, column=0, padx=5, pady=5, sticky=W)
        self.__dms_password_entry_var = StringVar()
        self.__dms_password_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_password_entry_var, show='*',
                                          state=initial_state)
        self.__dms_password_entry.grid(row=4, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__dms_controls.append(self.__dms_password_label)
        self.__dms_controls.append(self.__dms_password_entry)

        self.__dms_datasource_label = Label(self.__dms_label_frame, text='Datasource:', state=initial_state)
        self.__dms_datasource_label.grid(row=5, column=0, padx=5, pady=5, sticky=W)
        self.__dms_datasource_entry_var = StringVar()
        self.__dms_datasource_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_datasource_entry_var,
                                            state=initial_state)
        self.__dms_datasource_entry.grid(row=5, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__dms_controls.append(self.__dms_datasource_label)
        self.__dms_controls.append(self.__dms_datasource_entry)

        self.__dms_folderid_label = Label(self.__dms_label_frame, text='Folder ID:', state=initial_state)
        self.__dms_folderid_label.grid(row=6, column=0, padx=5, pady=5, sticky=W)
        self.__dms_folderid_entry_var = StringVar()
        self.__dms_folderid_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_folderid_entry_var,
                                          state=initial_state)
        self.__dms_folderid_entry.grid(row=6, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__dms_controls.append(self.__dms_folderid_label)
        self.__dms_controls.append(self.__dms_folderid_entry)

        self.__dms_docid_label = Label(self.__dms_label_frame, text='Document ID:', state=initial_state)
        self.__dms_docid_label.grid(row=6, column=0, padx=5, pady=5, sticky=W)
        self.__dms_docid_entry_var = StringVar()
        self.__dms_docid_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_docid_entry_var,
                                          state=initial_state)
        self.__dms_docid_entry.grid(row=6, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__dms_controls.append(self.__dms_docid_label)
        self.__dms_controls.append(self.__dms_docid_entry)

        self.__dms_appworkspace_label = Label(self.__dms_label_frame, text='App Workspace:', state=initial_state)
        self.__dms_appworkspace_label.grid(row=7, column=0, padx=5, pady=5, sticky=W)
        self.__dms_appworkspace_entry_var = StringVar()
        self.__dms_appworkspace_entry = Entry(self.__dms_label_frame, textvariable=self.__dms_appworkspace_entry_var,
                                              state=initial_state)
        self.__dms_appworkspace_entry.grid(row=7, column=1, padx=5, pady=5, sticky=NSEW)
        self.__dms_appworkspace_browse_button = Button(self.__dms_label_frame, text='...', state=initial_state,
                                                       command=self.__dms_appworkspace_browse_callback)
        self.__dms_appworkspace_browse_button.grid(row=7, column=2, padx=5, pady=5, sticky=NSEW)
        self.__dms_controls.append(self.__dms_appworkspace_label)
        self.__dms_controls.append(self.__dms_appworkspace_entry)
        self.__dms_controls.append(self.__dms_appworkspace_browse_button)

        self.__dms_label_frame.columnconfigure(1, weight=1)


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


    def __has_managed_workspace_checkbutton_callback(self):
        if self.__has_managed_workspace_checkbutton_var.get():
            new_state = 'normal'
        else:
            new_state = 'disabled'
        for control in self.__dms_controls:
            control.config(state=new_state)


    def __dms_library_browse_callback(self):
        path = tkFileDialog.askopenfilename(initialdir='.', title='Find iModelDmsSupportB02.dll...',
                                            filetypes=[('Executable Files', '*.exe')])
        if (path):
            self.__dms_library_entry_var.set(path.replace('/', '\\'))


    def __dms_workspace_dir_browse_callback(self):
        path = tkFileDialog.askdirectory(initialdir='.', title='Select workspace directory...')
        if path:
            self.__dms_workspace_dir_entry_var.set(path.replace('/', '\\'))


    def __dms_appworkspace_browse_callback(self):
        path = tkFileDialog.askdirectory(initialdir='.', title='Select app workspace directory...')
        if path:
            self.__dms_workspace_dir_entry_var.set(path.replace('/', '\\'))


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
        if self.__debug_cfg_checkbutton_var.get() == 1:
            command_string += ' --DGN_DEBUGCFG'
        if self.__has_managed_workspace_checkbutton_var.get() == 1:
            command_string += ' --dms-library="{lib}" --dms-workspaceDir="{workspace_dir}" --dms-user="{user}" ' \
                              '--dms-password="{password}" --dms-datasource="{datasource}" ' \
                              '--dms-folderId={folder_id} --dms-documentId={doc_id} ' \
                              '--dms-appWorkspace="{app_workspace}"'.format(
                lib=self.__dms_library_entry_var.get(),
                workspace_dir=self.__dms_workspace_dir_entry_var.get(),
                user=self.__dms_user_entry_var.get(),
                password=self.__dms_password_entry_var.get(),
                datasource=self.__dms_datasource_entry_var.get(),
                folder_id=self.__dms_folderid_entry_var.get(),
                doc_id=self.__dms_docid_entry_var.get(),
                app_workspace=self.__dms_appworkspace_entry_var.get())
        return command_string


    def __execute_command_string(self, command_string):
        security_attributes = win32security.SECURITY_ATTRIBUTES()
        security_attributes.bInheritHandle = True
        startup_info = win32process.STARTUPINFO()
        process_info = win32process.CreateProcess(None, command_string, None, None, True, win32con.CREATE_NEW_CONSOLE,
                                                  None, None, startup_info)
        process_handle = process_info[0]
        return_code = win32event.WaitForSingleObject(process_handle, win32event.INFINITE)
        if return_code == win32event.WAIT_OBJECT_0:
            publisher_error_code = win32process.GetExitCodeProcess(process_handle)
        else:
            publisher_error_code = 0x8000
        return publisher_error_code


if __name__ == '__main__':
    main_window = MainWindow()
    main_window.show_dialog()
