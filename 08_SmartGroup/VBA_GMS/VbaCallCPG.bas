Attribute VB_Name = "VbaCallCPG"
Private Declare PtrSafe Function vbadll Lib "C:\Program Files\Corel\CorelDRAW Graphics Suite 2020\Draw\Plugins64\lycpg64.cpg" (ByVal code As Long) As Long

Sub VBA����CPG_CDR���������AI()
 ret = vbadll(2)
 MsgBox "CDR���������AI ���� " & ret
End Sub

Sub AI���������CDR()
 ret = vbadll(1)
 MsgBox "AI���������CDR ���� " & ret
End Sub

Sub Clear_Fill()
 ret = vbadll(3)
 MsgBox "Clear_Fill���� " & ret
End Sub


Sub cql_FillColor()
 ret = vbadll(5)
End Sub


Sub Shapes_Filp()
 ret = vbadll(4)
End Sub

Sub cql_OutlineColor()
 ret = vbadll(6)
End Sub

Sub cql_SameSize()
 ret = vbadll(7)
End Sub

Sub fill_red()
 ret = vbadll(8)
End Sub
