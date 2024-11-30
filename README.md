# MNSC
### スクリプト実行用 DLL (pascal系)

レジストリ不要でDLLのコピーのみで動作する。  
アプリケーションで発生するイベントを処理するのが目的。  
EXE側でobjectを定義作成し、スクリプト側で操作できる。  
IDispatchを使用しない、簡易なInvoke機構を使用する。  
下向き構文解析でツリーを維持したまま実行するタイプ。  
C++ 20。  
/MSSC, script dll  
/MFCEdit, simple script editor  

////////////////////////////////////////////////////////////////////  
// sample1.  
func main()  
var a,b,c;  
begin  
&emsp; a = '{ "key1": "value1", "key2":[1,2,3] }'; // json  
  &emsp;  print(a);    
&emsp;b = json_parse(a);  
  &emsp;  print(b["key2"][0]); // 1  
 &emsp;   c = json_stringify(b);  
 &emsp;   print(c);  
&emsp;   return "ok";  
end  
