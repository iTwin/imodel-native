using System;
using System.Windows.Input;

namespace ORDBridgeGUI
    {
    class RelayCommand : ICommand
        {
        #region Fields
        private Action<object> m_executeWithArg;
        private Predicate<object> m_canExecute;
        private event EventHandler m_CanExecuteChangedInternal;
        #endregion // Fields

        #region Constructors
        //---------------------------------------------------------------------------------------
        // Creates a new command.
        // <param name="execute">The execution logic.</param>
        // <param name="canExecute">The execution status logic.</param>
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
        public RelayCommand (Action<object> execute, Predicate<object> canExecute)
            {
            m_executeWithArg = execute ?? throw new ArgumentNullException("execute");
            m_canExecute = canExecute ?? throw new ArgumentNullException("canExecute");

            //if ( execute == null )
            //    throw new ArgumentNullException("execute");

            //if ( canExecute == null )
            //    throw new ArgumentNullException("canExecute");

            //m_executeWithArg = execute;
            //m_canExecute = canExecute;
            }

        //---------------------------------------------------------------------------------------
        // Creates a new command that can always execute.
        // <param name="execute">The execution logic.</param>
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
        public RelayCommand (Action<object> execute)
            : this(execute, DefaultCanExecute)
            {
            }
        #endregion // Constructors

        public event EventHandler CanExecuteChanged
            {
            add
                {
                CommandManager.RequerySuggested += value;
                m_CanExecuteChangedInternal += value;
                }

            remove
                {
                CommandManager.RequerySuggested -= value;
                m_CanExecuteChangedInternal -= value;
                }
            }

        public void Execute (object parameter)
            {
            m_executeWithArg(parameter);
            }

        public bool CanExecute (object parameter)
            {
            return m_canExecute == null ? true : m_canExecute(parameter);
            }

        public void OnCanExecuteChanged ()
            {
            EventHandler handler = m_CanExecuteChangedInternal;
            if ( handler != null )
                {
                handler.Invoke(this, EventArgs.Empty);
                }
            }

        private static bool DefaultCanExecute (object param)
            {
            return true;
            }
        }
    }
