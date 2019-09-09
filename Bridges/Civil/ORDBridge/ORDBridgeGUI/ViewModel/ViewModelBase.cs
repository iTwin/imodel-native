/*--------------------------------------------------------------------------------------+
|
|     $Source: ORDBridge/ORDBridgeGUI/ViewModel/ViewModelBase.cs $
|
|  $Copyright: (c) 2019 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

using System;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.CompilerServices;

namespace ORDBridgeGUI.ViewModel
    {

    public abstract class ViewModelBase : INotifyPropertyChanged
        {

        #region Constructor

        //---------------------------------------------------------------------------------------
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
        protected ViewModelBase ()
            {
            }

        #endregion // Constructor

        #region Debugging Aides

        //---------------------------------------------------------------------------------------
        // Warns the developer if this object does not have a public property with the specified 
        // name. This method does not exist in a Release build.
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
      [Conditional("DEBUG")]
        [DebuggerStepThrough]
        public void VerifyPropertyName (string propertyName)
            {
            // Verify that the property name matches a real, public, instance property on this object.
            if ( TypeDescriptor.GetProperties(this)[propertyName] == null )
                {
                string msg = "Invalid property name: " + propertyName;

                if ( this.ThrowOnInvalidPropertyName )
                    throw new Exception(msg);
                else
                    Debug.Fail(msg);
                }
            }

        //---------------------------------------------------------------------------------------
        // Returns whether an exception is thrown, or if a Debug.Fail() is used when an invalid
        // property name is passed to the VerifyPropertyName method.
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
        protected virtual bool ThrowOnInvalidPropertyName
            {
            get; private set;
            }

        #endregion // Debugging Aides

        #region INotifyPropertyChanged Members

        //---------------------------------------------------------------------------------------
        // Raised when a property on this object has a new value.
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
        public event PropertyChangedEventHandler PropertyChanged;

        //---------------------------------------------------------------------------------------
        // Raises this object's PropertyChanged event.
        // @bsimethod                           Arun.George                             11/2018
        //---------------------------------------------------------------------------------------
        protected virtual void OnPropertyChanged (string propertyName)
            {
            this.VerifyPropertyName(propertyName);

            PropertyChangedEventHandler handler = this.PropertyChanged;
            if ( handler != null )
                {
                var e = new PropertyChangedEventArgs(propertyName);
                handler(this, e);
                }
            }
        //public void RaisePropertyChangedEvent (string propertyName = null)
        public void RaisePropertyChangedEvent ([CallerMemberName] string propertyName = null)
            {
            PropertyChanged?.Invoke (this, new PropertyChangedEventArgs (propertyName));
            //if ( PropertyChanged != null )
            //    PropertyChanged(this, new PropertyChangedEventArgs(propertyName));
            }

        #endregion // INotifyPropertyChanged Members

        }
    }
