package com.bentley.loadprojects;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.os.StrictMode;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.util.Log;
import java.util.ArrayList;
import java.util.List;
import android.view.View;
import android.widget.ListView;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MyActivity";
    private ListView projectListView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        if (android.os.Build.VERSION.SDK_INT > 9) {
            StrictMode.ThreadPolicy policy =
                    new StrictMode.ThreadPolicy.Builder().permitAll().build();
            StrictMode.setThreadPolicy(policy);
        }

        projectListView = (ListView)findViewById(R.id.projectList);
        projectListView.setVisibility(View.INVISIBLE);
        ConnectInterface.Initialize(this);
    }

    public void onExecute(View view){
        // Example of a call to a native method
        TextView tv = (TextView) findViewById(R.id.sample_text);
        //get the user name;
        /*String name = ConnectInterface.GetConnectUser();
        tv.setText("UserName: " + name);*/

        //get the project count
        List<String> list = ConnectInterface.GetProjects();
        tv.setText("Project Count: " + list.size());
        if (list.size() > 0)  {
            projectListView.setVisibility(View.VISIBLE);
            ArrayAdapter adapter = new ArrayAdapter<String>(
                    this,
                    android.R.layout.simple_list_item_1,
                    android.R.id.text1,
                    list);
            projectListView.setAdapter(adapter);
            projectListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    //ListView clicked item index
                    int itemPosition =  position;
                    //ListView clicked item value
                    String itemValue = (String) projectListView.getItemAtPosition(position);
                    //Show Alert
                    Toast.makeText(
                            getApplicationContext(),
                            "Position :"+itemPosition+"  ListItem : " +itemValue,
                            Toast.LENGTH_LONG).show();

                    Intent intent = new Intent(getApplicationContext(), ProjectDetailsActivity.class);
                    startActivity(intent);
                }
            });
        }
    }

    // Used to load the 'native-lib' library on application startup.
    static {
        Log.d(TAG, "Loading main activity...");
    }
}
