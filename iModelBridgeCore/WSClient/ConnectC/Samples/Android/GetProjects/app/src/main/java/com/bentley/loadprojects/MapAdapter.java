package com.bentley.loadprojects;

import android.content.Context;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Map;

public class MapAdapter extends BaseAdapter {
    private final ArrayList mData;
    private int  m_resource = 0;
    private int m_keyTextViewResourceId = 0;
    private int m_valueTextViewResourceId = 0;

    public MapAdapter(Context context, int resource, int keyTextViewResourceId, int valueTextViewResourceId, Map<String, String> map) {
        m_resource = resource;
        m_keyTextViewResourceId = keyTextViewResourceId;
        m_valueTextViewResourceId = valueTextViewResourceId;
        mData = new ArrayList();
        mData.addAll(map.entrySet());

    }

    @Override
    public int getCount() {
        return mData.size();
    }

    @Override
    public Map.Entry<String, String> getItem(int position) {
        return (Map.Entry) mData.get(position);
    }

    @Override
    public long getItemId(int position) {
        // TODO implement you own logic with ID
        return position;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {
        final View result;

        if (convertView == null) {
            result = LayoutInflater.from(parent.getContext()).inflate(m_resource, parent, false);
        } else {
            result = convertView;
        }

        Map.Entry<String, String> item = getItem(position);

        if (m_keyTextViewResourceId > 0)
            ((TextView) result.findViewById(m_keyTextViewResourceId)).setText(item.getKey());

        if (m_valueTextViewResourceId > 0)
            ((TextView) result.findViewById(m_valueTextViewResourceId)).setText(item.getValue());

        return result;
    }
}
