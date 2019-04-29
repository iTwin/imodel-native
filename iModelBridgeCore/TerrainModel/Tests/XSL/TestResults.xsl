<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:fo="http://www.w3.org/1999/XSL/Format">
<xsl:output encoding="UTF-8"/>
	<xsl:template match="/">
		<html>
			<head>
				<title>Test Results Summary</title>
			</head>
			<body>
				<h2>Test Results Summary</h2>
				<table border="1">
					<tr>
						<th align="left">Date</th>
						<th align="left">Unit Test Name</th>
						<th align="left">DGN</th>
						<th align="right">Total Tests</th>
						<th align="right">Failures</th>
						<th align="right">Not-run</th>
					</tr>
					<xsl:apply-templates select="full-test-results/test-results"/>
				</table>
			</body>
		</html>
	</xsl:template>
	<xsl:template match="test-results">
		<!-- <test-results name="D:\msl\arenium\dotNet\Server\ARWebService\WebServiceTests\bin\WebServiceTests.dll" total="4" failures="2" not-run="0" date="13/08/2004" time="12:54"> -->
		<tr>
			<td>
				<xsl:value-of select="@date"/>
			</td>
			<td>
				<xsl:value-of select="@name"/>
			</td>
			<td>
				<xsl:value-of select="@testedDGN"/>
			</td>
			<td align="right">
				<xsl:value-of select="@total"/>
			</td>
			<xsl:choose>
				<xsl:when test="@failures!='0'">
					<td align="right"  bgcolor="#FF0000">
						<xsl:value-of select="@failures"/>
					</td>
				</xsl:when>
				<xsl:otherwise>
					<td align="right">
						<xsl:value-of select="@failures"/>
					</td>
				</xsl:otherwise>
			</xsl:choose>
			<xsl:choose>
				<xsl:when test="@not-run!='0'">
					<td align="right"  bgcolor="#FF0000">
						<xsl:value-of select="@not-run"/>
					</td>
				</xsl:when>
				<xsl:otherwise>
					<td align="right">
						<xsl:value-of select="@not-run"/>
					</td>
				</xsl:otherwise>
			</xsl:choose>
		</tr>
	</xsl:template>
</xsl:stylesheet>
