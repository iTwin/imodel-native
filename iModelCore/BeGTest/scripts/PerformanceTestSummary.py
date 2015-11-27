import sys
import xlsxwriter
import csv
import math

counter = 0

# Class that holds results from both runs. Have one entry each in tr
class TestResults:
    def __init__(self):
        self.TCName = ""
        self.TName = ""
        self.TDesc = ""
        self.Speed_c = [] #Current
        self.Speed_b = [] #Benchmark

    def setValues(self, TCName, TName, TDesc):
        self.TCName = TCName
        self.TName = TName
        self.TDesc = TDesc

    def addSpeed(self, speed, curr):
        if curr:
            self.Speed_c.append(speed)
        else:
            self.Speed_b.append(speed)

    def getAvgSpeed(self, curr):
        if curr:
            totalSpeed = 0.00
            count = len(self.Speed_c)
            if count != 0:
                for i in range(count):
                    totalSpeed = totalSpeed + self.Speed_c[i]
                return totalSpeed / count
            else:
                return 0
        else:
            totalSpeed = 0.00
            count = len(self.Speed_b)
            if count != 0:
                for i in range(count):
                    totalSpeed = totalSpeed + self.Speed_b[i]
                return totalSpeed / count
            else:
                return 0
    def getStdDev(self, curr):
        if curr:
            count = len(self.Speed_c)
            sum = 0.0
            if count != 0:
                for i in range(count):
                    # Sum of (x - avg)'s square
                    time1 = self.Speed_c[i] - self.getAvgSpeed(True)
                    time2 = time1 * time1
                    sum = sum + time2
                val = sum / (count - 1)
                return math.sqrt(val)
            else:
                return 0
        else:
            count = len(self.Speed_b)
            sum = 0.0
            if count != 0:
                for i in range(count):
                    # Sum of (x - avg)'s square
                    time1 = self.Speed_b[i] - self.getAvgSpeed(False)
                    time2 = time1 * time1
                    sum = sum + time2
                val = sum / (count -1)
                return math.sqrt(val)
            else:
                return 0

    def getPercentStdError(self, curr):
        return self.getStdError(curr) / self.getAvgSpeed( curr )
    
    def getStdError (self, curr):
        if curr:
            count = len(self.Speed_c)
        else:
            count = len(self.Speed_b)            
        if count != 0:
            return self.getStdDev(curr) / math.sqrt(count)
        else:
            return 0
            
    def shouldAdd (self, curr):
        if self.getAvgSpeed(curr) == 0.0: # All times are Zero
                return False
        return True
        error = self.getPercentStdError(curr)
        if error >= 5:
            return False
        else:
            return True

tr = [TestResults() for i in range(2000)]


def findTestResult(toFind):
    found = -1
    for position, t in enumerate(tr):
        vals = t.TCName + t.TName + t.TDesc
        if vals == toFind:
            found = position
    return found

def addHeader(ws, maxRun):
    ws.set_column(1,1,50)
    ws.set_column(2,2,30)
    ws.write('A1', 'S. No.')
    ws.write('B1', 'Test Class')
    ws.write('C1', 'Test Name')
    ws.write('D1', 'Test Description')
    ws.write('G1', 'Standard Error')
    ws.write('H1', 'Standard Deviation')
    ws.write('E1', 'Average Speed')
    ws.write('F1', '% Standard Error')
    for i in range(0, maxRun):
        ws.write(0, 8 + i, "Run" + str(i+1))


def addData (CSV, ws, curr):
    s_maxRun = 0
    global counter
    
    percentFormat = wb.add_format()
    percentFormat.set_num_format('0%')

    numFormat = wb.add_format()
    numFormat.set_num_format('#,##0')

    ws.set_column(4,7,15)
    
    # Add entries
    csvfile = open(CSV, 'r')
    reader = csv.reader(csvfile, delimiter=',')
    reader.next() # skip header
    for row in reader:
        testToFind = row[1].strip() + row[2].strip() + row[4].strip()
        position = findTestResult(testToFind)
        if position == -1: # Test entry doesn't exist, we need to add a new entry
            tr[counter].setValues(row[1].strip(), row[2].strip(), row[4].strip())
            if float(row[3].strip()) != 0.0 and (row[5].strip()).isdigit():
                tr[counter].addSpeed(float(row[5].strip()) / float(row[3].strip()), curr)
            counter = counter + 1
        else:                           # Test is there, just add it's time
            if float(row[3].strip()) != 0.0 and (row[5].strip()).isdigit():
                tr[position].addSpeed(float(row[5].strip()) / float(row[3].strip()), curr)
    sNo = 0
    rejected = 0
    for i in range(counter):
        if tr[i].shouldAdd(curr):
            sNo = sNo + 1
            ws.write(sNo, 0, sNo)                   # S. No.
            ws.write(sNo, 1, tr[i].TCName)          # Test Case Name
            ws.write(sNo, 2, tr[i].TName)           # Test Name
            ws.write(sNo, 3, tr[i].TDesc)           # Test Description
            if curr: # this is current sheet
                speedCount_c = len(tr[i].Speed_c)
                if speedCount_c > 0: #it's current entries are there
                    for j in range (speedCount_c):
                        ws.write(sNo, 8 + j, tr[i].Speed_c[j])                       # Speed
                    ws.write(sNo, 4, tr[i].getAvgSpeed(curr), numFormat)             # Average Speed
                    ws.write(sNo, 5, tr[i].getPercentStdError(curr) , percentFormat) # % Standard Error
                    ws.write(sNo, 6, tr[i].getStdError(curr), numFormat)             # Standard Error
                    ws.write(sNo, 7, tr[i].getStdDev(curr), numFormat)               # Standard Deviation
                    #set header lenght
                    if s_maxRun < speedCount_c:
                        s_maxRun = speedCount_c
            else:
                speedCount_b = len(tr[i].Speed_b)
                if speedCount_b > 0: #it's benchmark entries are there
                    for j in range (speedCount_b):
                        ws.write(sNo, 8 + j, tr[i].Speed_b[j])                      # Speed
                    ws.write(sNo, 4, tr[i].getAvgSpeed(curr), numFormat)            # Average Speed
                    ws.write(sNo, 5, tr[i].getPercentStdError(curr), percentFormat) # % Standard Error
                    ws.write(sNo, 6, tr[i].getStdError(curr), numFormat)            # Standard Error
                    ws.write(sNo, 7, tr[i].getStdDev(curr), numFormat)              # Standard Deviation
                    #set header lenght
                    if s_maxRun < speedCount_b:
                        s_maxRun = speedCount_b
        else:
            rejected = rejected + 1
    #Add header
    addHeader(ws, s_maxRun)
    print "*** INFO: Added Entries: " + str(sNo)
    print "*** INFO: Rejected Entries: " + str(rejected)
    
def addSummary(ws):
    global wb, counter, range
    #global currStream, currDate, benStream, benDate
    
    heading = wb.add_format()
    heading.set_bold()
    heading.set_font_size(16)
    heading.set_font_color('brown')
    heading.set_align('center')

    heading2 = wb.add_format()
    heading2.set_bold()
    heading2.set_font_size(12)
    heading2.set_font_color('blue')
    heading2.set_align('center')
    
    blue = wb.add_format()
    blue.set_bold()
    blue.set_font_color('blue')

    green = wb.add_format()
    green.set_bold()
    green.set_font_color('green')

    red = wb.add_format()
    red.set_bold()
    red.set_font_color('red')

    orange = wb.add_format()
    orange.set_bold()
    orange.set_font_color('orange')

    numFormat = wb.add_format()
    numFormat.set_num_format('#,##0')

    percentFormat = wb.add_format()
    percentFormat.set_num_format('0%')

    bold = wb.add_format({'bold': True})
    bold.set_align('center')
    bold.set_border()
    right = wb.add_format()
    right.set_align('right')

    ws.set_column(9,10,15)

    ws.set_row(0,20);
    ws.merge_range ('A1:H1', 'DgnDb Performance Tests Summary', heading)
    # Add any machine specific info here
    #ws.merge_range ('A2:H2', 'Azur VM D3 (4 Core Processor, RAM 14GB, HDD 200 GB SSD)', heading2)
    ws.merge_range ('A3:H3', 'Note: The Benchmark and Current columns list Average Speed (Operations / Second) and % Standard Error for the Average Speed. The Stream and Date is also listed. Rows are color coded as below')
    ws.write ('B4', 'Severe Degradation (>33%)', red)
    ws.write ('C4', 'Mild Degradation (>10%)', orange)
    ws.write ('E4', 'Mild Improvement (<-10%)', blue)
    ws.write ('G4', 'Great Improvement ( <-33%)', green)

    #Current and becnhmark details
    # Modify the headers for summary
    currStream = "DgnDb06Dev"
    currDate   = ""
    benStream  = "DgnDb06Dev"
    benDate    = ""

    ws.merge_range('D5:E5', 'Current Run', bold)
    ws.merge_range('F5:G5', 'Benchmark Run', bold)
    ws.merge_range('D6:E6', currStream + " - " + currDate, bold)
    ws.merge_range('F6:G6', benStream  + " - " + benDate, bold)

    ws.set_column(0,0,5)
    ws.set_column(1,2,35)
    ws.set_column(3,3,18)
    ws.set_column(4,5,12)
    ws.set_column(6,6,18)
    ws.set_column(7,7,60)

    rowNo = 6
    ws.write(rowNo, 0, 'S. No.', bold)
    ws.write(rowNo, 1, 'Test Class', bold)
    ws.write(rowNo, 2, 'Test Name', bold)
    ws.write(rowNo, 3, '% Standard Error', bold)
    ws.write(rowNo, 4, 'Speed', bold)    
    ws.write(rowNo, 5, 'Speed', bold)
    ws.write(rowNo, 6, '% Standard Error', bold)
    ws.write(rowNo, 7, 'Test Description', bold)

    ws.freeze_panes(rowNo + 1, 0)
    ws.autofilter('A7:H7')
    
    #Write data
    sNo = 0
    for i in range(counter):
        if (tr[i].shouldAdd(True) and tr[i].shouldAdd(False)):
            sNo = sNo + 1
            ws.write(rowNo + sNo, 0, sNo)                   # S. No.
            ws.write(rowNo + sNo, 1, tr[i].TCName)          # Test Case Name
            ws.write(rowNo + sNo, 2, tr[i].TName)           # Test Name
            ws.write(rowNo + sNo, 7, tr[i].TDesc)           # Test Description
            speedCount_c = len(tr[i].Speed_c)
            if speedCount_c > 0: #it's current times are there
                ws.write(rowNo + sNo, 3, tr[i].getPercentStdError(True), percentFormat)    # % Standard Error for Current Run
                ws.write(rowNo + sNo, 4, tr[i].getAvgSpeed(True), numFormat)    # Average Current Speed
            speedCount_b = len(tr[i].Speed_b)
            if speedCount_b > 0: #it's benchmark times are there
                ws.write(rowNo + sNo, 6, tr[i].getPercentStdError(False), percentFormat)    # % Standard Error for Benchmark Run
                ws.write(rowNo + sNo, 5, tr[i].getAvgSpeed(False), numFormat)    # Average Benchmark Speed

    #apply conditional formatting
    print '*** INFO: Applying formatting'
    fromRow = rowNo + 2
    tillRow = rowNo + sNo + 1
    range = 'A' + str(fromRow) + ':G' + str(tillRow)
    
    ws.conditional_format(range, {'type':     'formula',
                                        'criteria': '=AND((((($E8-$F8)/$F8)*100)<=-10), (((($E8-$F8)/$F8)*100)>-33))',
                                        'format':   orange})

    ws.conditional_format(range, {'type':     'formula',
                                        'criteria': '=((($E8-$F8)/$F8)*100)< -33',
                                        'format':   red})

    ws.conditional_format(range, {'type':     'formula',
                                        'criteria': '=AND((((($E8-$F8)/$F8)*100)>=10), (((($E8-$F8)/$F8)*100)<33))',
                                        'format':   blue})

    ws.conditional_format(range, {'type':     'formula',
                                        'criteria': '=((($E8-$F8)/$F8)*100)>33',
                                        'format':   green})

def Help():
    print '***Example use:'
    print '***PerformanceTestSummary.py --current=Current.csv --benchmark=Benchmark.csv --output=Analysis.xlsx'
    print '***Where Current.csv is the current run and Benchmark.csv is the run to compare with. Results stored in Analysis.xlsx'
    print '***If you skip the benchmark, it will just create sheet for current run'
    
def GetArgument(needed):
    found = 0
    for i in range(1,4):
        arg = sys.argv[i]
        if arg.startswith(needed) and found == 0:
            found = 1
            argParts = arg.split("=")
            return argParts[1]
    print '***ERROR: Parameter not defined: ' + needed
    print Help()
    sys.exit()
            

# Entry point of the script
if (len(sys.argv) < 2):
    print '***ERROR: No parameters were provided.'
    print Help()
    sys.exit()
    
else:
    # Create Excel workbook and sheets
    wb = xlsxwriter.Workbook(GetArgument("--output="))
    if (len(sys.argv) > 3):
        wsSummary = wb.add_worksheet('Summary')
        wsBench = wb.add_worksheet('Benchmark')
    wsCurrent = wb.add_worksheet('Current')

    print '*** INFO: Populating sheet with current run'
    curr = GetArgument("--current")
    addData (curr, wsCurrent, True)
    
    if len(sys.argv) > 3:
        bench = GetArgument("--benchmark")
        print '*** INFO: Populating sheet with benchmark run'
        addData (bench, wsBench, False)
        print '*** INFO: Populating Summary sheet'
        addSummary(wsSummary)
    print '*** INFO: Done.'
    wb.close()

