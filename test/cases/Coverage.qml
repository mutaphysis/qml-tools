import QtQuick 2.0

Item {
    id: root
    width: 100
    height: 100
    
    property int value: { if (test > 5) { 1 } else { 2 } }
    property int test: 1
}