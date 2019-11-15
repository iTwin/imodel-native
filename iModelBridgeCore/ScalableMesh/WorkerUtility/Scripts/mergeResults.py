#---------------------------------------------------------------------------------------------
#  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
#  See COPYRIGHT.md in the repository root for full copyright notice.
#---------------------------------------------------------------------------------------------
import sys
import csv
import os


tables = {}
sourceDir = sys.argv[1]
for f in os.listdir(sourceDir):
    if os.path.splitext(f)[1] == ".csv" :
        tables[f] = []
        with open(os.path.join(sourceDir,f), "rb") as csv_file:
            csv_reader = csv.reader((line.replace('\0','') for line in csv_file), delimiter=',')
            has_read_columns = False
            current_table_idx = 0
            for row in csv_reader:
                if not has_read_columns:
                    obj = {}
                    obj["columns"] = row
                    for col in row:
                        obj[col] = []
                    tables[f].append(obj)
                    has_read_columns = True
                else:
                    if len(row) > 0:
                        for i in range(0, len(row)):
                            tables[f][current_table_idx][tables[f][current_table_idx]["columns"][i]].append(row[i])
                    else:
                        has_read_columns = False
                        current_table_idx = current_table_idx+1

max_stop_time = -1
min_start_time = -1

taskList = []

for f in tables.iterkeys():
    names=f.split("_")
    if len(tables[f]) >= 1 and "StopTime" in tables[f][0].iterkeys():
        st = tables[f][0]["StopTime"]
        if max_stop_time == -1: max_stop_time = int(st[0])
        else: max_stop_time = max(max_stop_time, int(st[0]))
    if len(tables[f]) >= 1 and "StartTime" in tables[f][0].iterkeys():
        st = tables[f][0]["StartTime"]
        if min_start_time == -1: min_start_time = int(st[0])
        else: min_start_time = min(min_start_time, int(st[0]))

    if len(tables[f]) >= 2:
        if len(taskList) == 0:
            taskList.append(tables[f][1]["columns"])
        if len(taskList[0]) > 0:
            for j in range(0, len(tables[f][1][taskList[0][i]])):
                row = []
                row.append(names[-3]+"_"+names[-2])
                for i in range(0, len(taskList[0])):
                    row.append(tables[f][1][taskList[0][i]][j])
                taskList.append(row)
taskList[0].insert(0,"Name")

table1 = [["Total Start Time", "Total Stop Time", "Duration (s)"],[min_start_time, max_stop_time, max_stop_time-min_start_time]]

with open(os.path.join(sourceDir,'final.csv'), mode='w') as csv_out_file:
    csv_writer = csv.writer(csv_out_file, delimiter=',', quotechar='"', quoting=csv.QUOTE_MINIMAL)
    csv_writer.writerow(table1[0])
    csv_writer.writerow(table1[1])
    csv_writer.writerow(["  "])
    for rowidx in range(0, len(taskList)):
        print taskList[rowidx]
        csv_writer.writerow(taskList[rowidx])
    	