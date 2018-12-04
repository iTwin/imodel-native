package com.bentley.loadprojects;

import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.widget.TextView;

import java.util.HashMap;

enum ProjectPropertyIds {
            PROJECT_OBJECTID        (1), /**< \b ObjectId. */
            PROJECT_ULTIMATEREFID   (2), /**< \b UltimateRefId. */
            PROJECT_ISRBACENABLED   (3), /**< \b IsRbacEnabled. */
            PROJECT_NAME            (4), /**< \b Name. */
            PROJECT_NUMBER          (5), /**< \b Number. */
            PROJECT_INDUSTRY        (6), /**< \b Industry. */
            PROJECT_ASSETTYPE       (7), /**< \b AssetType. */
            PROJECT_LASTMODIFIED    (8), /**< \b LastModified. */
            PROJECT_LOCATION        (9), /**< \b Location. */
            PROJECT_LATITUDE        (10), /**< \b Latitude. */
            PROJECT_LONGITUDE       (11), /**< \b Longitude. */
            PROJECT_LOCATIONISUSINGLATLONG (12), /**< \b LocationIsUsingLatLong. */
            PROJECT_REGISTEREDDATE  (13), /**< \b RegisteredDate. */
            PROJECT_TIMEZONELOCATION(14), /**< \b TimeZoneLocation. */
            PROJECT_STATUS          (15), /**< \b Status. */
            PROJECT_DATA_LOCATION_GUID   (16), /**< \b Data_Location_Guid. */
            PROJECT_COUNTRY_CODE    (17); /**< \b Country_Code. */

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
