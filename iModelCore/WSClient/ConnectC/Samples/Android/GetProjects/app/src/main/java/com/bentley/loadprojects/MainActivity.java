package com.bentley.loadprojects;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.os.StrictMode;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.TextView;
import android.util.Log;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import android.view.View;
import android.widget.ListView;
import android.widget.Toast;


public class MainActivity extends AppCompatActivity {

    private static final String TAG = "MyActivity";
    private TextView m_statusTextView;
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

        m_statusTextView = (TextView) findViewById(R.id.sample_text);
        projectListView = (ListView)findViewById(R.id.projectList);
        projectListView.setVisibility(View.INVISIBLE);
        ConnectInterface.Initialize(this);
    }

    @Override
    protected void onResume()
    {
        displayList();
        super.onResume();
    }

    private void displayList()
    {
        MyApplication app = (MyApplication)getApplicationContext();
        HashMap<String, String> projects  = app.GetProjects();

        if (projects.size() > 0) {
            m_statusTextView.setText("Project Count: " + projects.size());

            MapAdapter adapter = new MapAdapter(this, android.R.layout.simple_list_item_1,
                    0, android.R.id.text1, projects);
            projectListView.setAdapter(adapter);
            projectListView.setVisibility(View.VISIBLE);
            projectListView.setOnItemClickListener(new AdapterView.OnItemClickListener() {
                @Override
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    //ListView clicked item index
                    int itemPosition =  position;
                    Object obj = projectListView.getItemAtPosition(position);
                    //ListView clicked item value
                    String itemKey  = (String) ((Map.Entry) projectListView.getItemAtPosition(position)).getKey();
                    String itemValue = (String) ((Map.Entry) projectListView.getItemAtPosition(position)).getValue();
                    //Show Alert
                    Toast.makeText(
                            getApplicationContext(),
                            "Position :"+itemPosition+"  ListItem : " +itemValue,
                            Toast.LENGTH_LONG).show();

                    Intent intent = new Intent(getApplicationContext(), ProjectDetailsActivity.class);
                    //pass the "id" to the next activity
                    //Since there is no real way to get the instance id from the data in CWSCC currently, just pass one we know is there - for sample purposes.
                    intent.putExtra("projid", itemKey);
                    startActivity(intent);
                }
            });
        }
    }

    private void retrieveProjectDataFromServer() {
        HashMap<String, String> projects = ConnectInterface.GetProjects();
        MyApplication app = (MyApplication)getApplicationContext();
        app.SetProjects(projects);
    }
    public void onExecute(View view){
        retrieveProjectDataFromServer();
        displayList();
        }

    // Used to load the 'native-lib' library on application startup.
    static {
        Log.d(TAG, "Loading main activity...");
    }
    }

