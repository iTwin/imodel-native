# --------------------------------------------------------------------------------------
#      $Source: ORDBridge/PythonScripts/PublishORDToHub.py $
#   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
#--------------------------------------------------------------------------------------

from Tkinter import *
import tkFileDialog
import ttk


class MainWindow:
    def __init__(self, root):
        self.__root = root
        self.__root.title('Publish ORD To iModel Hub')
        self.__initialize_controls()


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
                                               state='readonly', values=['QA', 'PROD'])
        self.__server_env_combo.grid(row=8, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__server_env_combo_var.set('QA')

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
            self.__fwk_exe_entry_var.set(path)


    def __bridge_lib_browse_callback(self):
        path = tkFileDialog.askopenfilename(initialdir='.', title='Find bridge library...',
                                            filetypes=[('Library Files', '*.dll')])
        if path:
            self.__bridge_lib_entry_var.set(path)


    def __staging_dir_browse_callback(self):
        path = tkFileDialog.askdirectory(initialdir='.', title='Select staging directory...')
        if path:
            self.__staging_dir_entry_var.set(path)


    def __input_file_browse_callback(self):
        path = tkFileDialog.askopenfilename(initialdir='.', title='Find input file...',
                                            filetypes=[('OpenRoads Designer Files', '*.dgn')])
        if (path):
            self.__input_file_entry_var.set(path)


    def __publish_button_callback(self):
        pass


if __name__ == '__main__':
    root = Tk()
    main_window = MainWindow(root)
    root.mainloop()
