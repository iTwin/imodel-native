package com.bentley.loadprojects;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.util.HashMap;

enum ProjectPropertyIds {
            PROJECT_ULTIMATEREFID   (1), /**< \b UltimateRefId. */
            PROJECT_ISRBACENABLED   (2), /**< \b IsRbacEnabled. */
            PROJECT_NAME            (3), /**< \b Name. */
            PROJECT_NUMBER          (4), /**< \b Number. */
            PROJECT_INDUSTRY        (5), /**< \b Industry. */
            PROJECT_ASSETTYPE       (6), /**< \b AssetType. */
            PROJECT_LASTMODIFIED    (7), /**< \b LastModified. */
            PROJECT_LOCATION        (8), /**< \b Location. */
            PROJECT_LATITUDE        (9), /**< \b Latitude. */
            PROJECT_LONGITUDE       (10), /**< \b Longitude. */
            PROJECT_LOCATIONISUSINGLATLONG (11), /**< \b LocationIsUsingLatLong. */
            PROJECT_REGISTEREDDATE  (12), /**< \b RegisteredDate. */
            PROJECT_TIMEZONELOCATION(13), /**< \b TimeZoneLocation. */
            PROJECT_STATUS          (14), /**< \b Status. */
            PROJECT_DATA_LOCATION_GUID   (15), /**< \b Data_Location_Guid. */
            PROJECT_COUNTRY_CODE    (16); /**< \b Country_Code. */

            private final int id;
            ProjectPropertyIds(int id) { this.id = id; }
            public int getValue() { return id; }
            }


public class ProjectDetailsActivity extends AppCompatActivity {

    private String m_instanceId = "";
    private TextView m_projNameTextView;
    private TextView m_projLocTextView;
    private TextView m_projLMDTextView;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_project_details);

        m_projNameTextView = (TextView) findViewById(R.id.val_proj_name);
        m_projLocTextView = (TextView) findViewById(R.id.val_proj_location);
        m_projLMDTextView = (TextView) findViewById(R.id.val_proj_lastmodifieddate);

        if (savedInstanceState == null) {
            Bundle extras = getIntent().getExtras();
            if(extras != null) {
                m_instanceId= extras.getString("projid");
            }
        } else {
            m_instanceId= (String) savedInstanceState.getSerializable("projid");
        }


        HashMap<Integer, String> properties = ConnectInterface.GetProjectProperties(m_instanceId);
        m_projNameTextView.setText(properties.get(ProjectPropertyIds.PROJECT_NAME.getValue()));
        m_projLocTextView.setText(properties.get(ProjectPropertyIds.PROJECT_LOCATION.getValue()));
        m_projLMDTextView.setText(properties.get(ProjectPropertyIds.PROJECT_LASTMODIFIED.getValue()));
    }

}
