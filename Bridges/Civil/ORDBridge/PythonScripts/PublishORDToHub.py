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
        self.__initialize_controls()


    def __initialize_controls(self):
        self.__fwk_exe_label = Label(self.__root, text='iModelBridgeFwk.exe Path:')\
            .grid(row=0, column=0, padx=5, pady=5, sticky=W)
        self.__fwk_exe_entry_var = StringVar()
        self.__fwk_exe_entry = Entry(self.__root, textvariable=self.__fwk_exe_entry_var)\
            .grid(row=0, column=1, padx=5, pady=5, sticky=NSEW)
        self.__fwk_exe_browse_button = Button(self.__root, text='...', command=self.__fwk_exe_browse_callback)\
            .grid(row=0, column=2, padx=5, pady=5)

        self.__bridge_lib_label = Label(self.__root, text='Bridge Library Path:')\
            .grid(row=1, column=0, padx=5, pady=5, sticky=W)
        self.__bridge_lib_entry_var = StringVar()
        self.__bridge_lib_entry = Entry(self.__root, textvariable=self.__bridge_lib_entry_var)\
            .grid(row=1, column=1, padx=5, pady=5, sticky=NSEW)
        self.__bridge_lib_browse_button = Button(self.__root, text='...', command=self.__bridge_lib_browse_callback)\
            .grid(row=1, column=2, padx=5, pady=5)

        self.__staging_dir_label = Label(self.__root, text='Staging Directory:')\
            .grid(row=2, column=0, padx=5, pady=5, sticky=W)
        self.__staging_dir_entry_var = StringVar()
        self.__staging_dir_entry = Entry(self.__root, textvariable=self.__staging_dir_entry_var)\
            .grid(row=2, column=1, padx=5, pady=5, sticky=NSEW)
        self.__staging_dir_browse_button = Button(self.__root, text='...', command=self.__staging_dir_browse_callback)\
            .grid(row=2, column=2, padx=5, pady=5)

        self.__input_file_label = Label(self.__root, text='Input File:').grid(row=3, column=0, padx=5, pady=5, sticky=W)
        self.__input_file_entry_var = StringVar()
        self.__input_file_entry = Entry(self.__root, textvariable=self.__input_file_entry_var)\
            .grid(row=3, column=1, padx=5, pady=5, sticky=NSEW)
        self.__input_file_browse_button = Button(self.__root, text='...', command=self.__input_file_browse_callback)\
            .grid(row=3, column=2, padx=5, pady=5)

        self.__server_project_label = Label(self.__root, text='CONNECT Project:')\
            .grid(row=4, column=0, padx=5, pady=5, sticky=W)
        self.__server_project_entry_var = StringVar()
        self.__server_project_entry = Entry(self.__root, textvariable=self.__server_project_entry_var)\
            .grid(row=4, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_repo_label = Label(self.__root, text='CONNECT iModel Name:')\
            .grid(row=5, column=0, padx=5, pady=5, sticky=W)
        self.__server_repo_entry_var = StringVar()
        self.__server_repo_entry = Entry(self.__root, textvariable=self.__server_repo_entry_var)\
            .grid(row=5, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_user_label = Label(self.__root, text='User Name:') \
            .grid(row=6, column=0, padx=5, pady=5, sticky=W)
        self.__server_user_entry_var = StringVar()
        self.__server_user_entry = Entry(self.__root, textvariable=self.__server_user_entry_var) \
            .grid(row=6, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_password_label = Label(self.__root, text='Password:') \
            .grid(row=7, column=0, padx=5, pady=5, sticky=W)
        self.__server_password_entry_var = StringVar()
        self.__server_password_entry = Entry(self.__root, textvariable=self.__server_password_entry_var, show='*') \
            .grid(row=7, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)

        self.__server_env_label = Label(self.__root, text='Server Environment:')\
            .grid(row=8, column=0, padx=5, pady=5, sticky=W)
        self.__server_env_combo_var = StringVar()
        self.__server_env_combo = ttk.Combobox(self.__root, textvariable=self.__server_env_combo_var,
                                               state='readonly', values=['QA', 'PROD'])\
            .grid(row=8, column=1, columnspan=2, padx=5, pady=5, sticky=NSEW)
        self.__server_env_combo_var.set('QA')

        self.__create_repo_checkbutton_var = IntVar()
        self.__create_repo_checkbutton = Checkbutton(self.__root, text='Create Repository (if necessary)?',
                                                     variable=self.__create_repo_checkbutton_var)\
            .grid(row=9, column=0, columnspan=3, padx=5, pady=5, sticky=W)
        self.__create_repo_checkbutton_var.set(1)

        self.__skip_assignment_check_checkbutton_var = IntVar()
        self.__skip_assignment_check_checkbutton = Checkbutton(self.__root, text='Skip Assignment Check?',
                                                               variable=self.__skip_assignment_check_checkbutton_var)\
            .grid(row=10, column=0, columnspan=3, padx=5, pady=5, sticky=W)
        self.__skip_assignment_check_checkbutton_var.set(1)

        self.__root.columnconfigure(1, weight=1)


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


if __name__ == '__main__':
    root = Tk()
    main_window = MainWindow(root)
    root.mainloop()
