import QtQuick 2.0


Item
/** comment **/
 { id: root; width: 100
    height: 100
    
    property int value: { if (test > 5) { 1 } else { 2 } } 
            property int value2: { 4 + 6 * 6 }
    property real value3
    property int test: 1; x: testFunc("hallo"); y: 10; property int value4: { 12 * "1" }
    
    function 
    testFunc() 
    {
        if (test > 5) {
            // some more comments
            return 12;
        }
        return test;
    }
}