import QtQuick 2.0

QtObject {
    id: root
    
    signal handler()
    
    function firstFunction() {
        return 5;
    }
    
    function secondFunction() { 
        return scriptProp + ternaryProp;        
    }
    
    property int staticProp: 4
    property alias aliasProp: root.staticProp
    property int functionProp: firstFunction()
    property int bindingProp: functionProp
    property int ternaryProp: x > y ? 34 : 12
    property int scriptProp: {
        if (Math.random() > 4) {
            return 32;
        } else {
            print("Wurstwasser");            
        }
        
        return 0;
    }
    overideProp: 5 * 9/3.0; onHandler: {}
    
    property var objectProp: QtObject { function inlineFunction() {} }
    property list<QtObject> arrayProp: [QtObject { function inlineListFunction() {} }, QtObject { }, QtObject { }]
    
    QtObject {
        function childFunction() {
        }
    }
}