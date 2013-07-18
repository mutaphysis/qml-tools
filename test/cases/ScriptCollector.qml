import QtQuick 2.0

QtObject  // {}
    /*   {}


{}*/ {
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
    property int functionCallProp: firstFunction()
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
    groupProp { one: 4; two: 4 * 2 * 1 }


    property var jsObjectProp: {
        "hello": "world",
        "today": 11.04
    }

    property var jsFunctionProp: function() {
    }

    property var objectProp: QtObject { function inlineFunction() {} }
    property list<QtObject> arrayProp: [QtObject { function inlineListFunction() {} }, QtObject { }, QtObject { }]

    Component.onSomeEventHandler: {
    }

    QtObject {
        function
    childFunction(param1, param2)
            {
        }
    }
}
