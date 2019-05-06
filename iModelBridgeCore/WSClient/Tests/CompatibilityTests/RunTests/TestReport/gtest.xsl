<?xml version='1.0'?>
<xsl:stylesheet
	version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
	<xsl:template match="testsuites">
		<html>
			<head>
				<title> gTest Report - <xsl:value-of select="@name"/> </title>
				<style type="text/css">
* {
    font-family:'Arial';
}

table * {
    word-break: break-all;
}
    </style>
			</head>

			<body bgcolor="#ffffff">
				<div align="center">
			        <h3>
            	        <b> gTest Report - <xsl:value-of select="@name"/> </b>
            		</h3>
                    
                    <h3>
                        <b>Ran <xsl:value-of select="@tests"/> tests and <xsl:value-of select="@disabled"/> tests were disabled</b>
                        <br/>
                        <b>Took <xsl:value-of select="@time"/> seconds or <xsl:value-of select="format-number(@time div 60,'0.00')"/> minutes</b>
                        <br/>
                        <xsl:choose>
                            <xsl:when test="@failures='0' and @errors='0'">
                                <b>No failures</b>
                            </xsl:when>
                            <xsl:otherwise>
                                <b><font color="red">
									Got 
									<xsl:value-of select="@failures"/> failures
									(of which
                                    <xsl:value-of select="count(//testsuite/testcase[contains(@name,'_KnownIssue') or contains(@value_param,'_KnownIssue')][failure])"/>
                                    known issues) and 
									<xsl:value-of select="@errors"/> errors
								</font></b>
                           </xsl:otherwise>
                        </xsl:choose>
                    </h3>
            
            	</div>
		        <table cols="4" width="100%" align="center" cellspacing="2" >
			        <tr>
				        <td width="120px"> </td>
				        <td> </td>
				        <td> </td>
				        <td width="120px"> </td>
        			</tr>
		        	<xsl:apply-templates/>
        		</table>
			</body>
		</html>
	</xsl:template>

	<xsl:template match="testsuite">
		<tr bgcolor="#008BE1" height="30">
			<td colspan="4">
				<font color="white"><b>Running Suite <xsl:value-of select="@name"/> </b></font>
			</td>
		</tr>
		<xsl:apply-templates/>
		<tr bgcolor="#ffffc0">
			<td colspan="4">
				Ran <xsl:value-of select="@tests"/> tests in <xsl:value-of select="@time"/> seconds,
				disabled: <xsl:value-of select="@disabled"/>,
				failures: <xsl:value-of select="@failures"/>,
				errors: <xsl:value-of select="@errors"/>
			</td>
		</tr>
	</xsl:template>

	<xsl:template match="testcase">
		<tr bgcolor="#e0f0d0">
			

			<xsl:choose>
                <xsl:when test="child::failure and (contains(@name,'_KnownIssue') or contains(@value_param,'_KnownIssue'))">
					<xsl:attribute name="bgcolor">#ff7b28</xsl:attribute>
                </xsl:when>
                <xsl:when test="child::failure">
					<xsl:attribute name="bgcolor">#ff5050</xsl:attribute>
                </xsl:when>
            </xsl:choose>
			
			<xsl:choose>
                <xsl:when test="child::failure and (contains(@name,'_KnownIssue') or contains(@value_param,'_KnownIssue'))">
                    <td> Failed<br/> (Known Issue) </td>
                </xsl:when>
                <xsl:when test="child::failure">
                    <td><b> Failed </b></td>
                </xsl:when>
                <xsl:when test="@status='notrun'">
                    <td bgcolor="#ffe66b"> Disabled </td>
                </xsl:when>
                <xsl:otherwise>
                    <td bgcolor="#50ff50"> Passed </td>
               </xsl:otherwise>
            </xsl:choose>
			
			<td colspan="3" >
				<xsl:value-of select="@name"/>
				<xsl:choose>
					<xsl:when test="@value_param">
						<br/>
						Param: <xsl:value-of select="@value_param"/>
					</xsl:when>
				</xsl:choose>
			</td>

		</tr>
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="failure">
		<tr bgcolor="#ffa500">
			<td align="right" valign="top" > File Name: </td>
			<td colspan="2">
				<xsl:value-of select="substring-before(.,':\')"/>:\<xsl:value-of select="substring-before(substring-after(.,':\'),':') "/>
			</td>
			<td>
				Line: <xsl:value-of select="substring-after(substring-before(substring-after(.,':\'),'&#x000A;'),':')"/>
			</td>
		</tr>
		<tr bgcolor="#ffa500">
			<td align="right" valign="top" > Message: </td>
			<td colspan="3">
				<div style="white-space: pre-wrap;font-family:'Lucida Console';font-size:12;">
                    <xsl:value-of select="substring-after(@message,'&#x000A;')"/>
                </div>
			</td>
		</tr>
	</xsl:template>
</xsl:stylesheet>
