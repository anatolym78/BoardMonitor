import QtQuick 2.12
import QtQuick.Controls 1.4 // Используем Controls 1.4 для TreeView в Qt 5
import QtQuick.Controls.Styles 1.4
import BoardStation 1.0

Rectangle
{
    anchors.fill: parent
    // // Создаем экземпляр нашей C++ модели
    // SimpleTreeModel
    // {
    //     id: myTreeModel
    // }
    //color: "gainsboro"

    TreeView
    {
        id: treeView
        anchors.fill: parent
        anchors.margins: 10
        model: simpleTreeModel
        headerVisible: true // Показываем заголовки колонок
        backgroundVisible: false
        alternatingRowColors: false
        frameVisible: false

        // Явно устанавливаем фокус на этот элемент
        focus: true
        clip: true
        // Принудительно делаем фокус активным после создания компонента
        Component.onCompleted:
        {
            treeView.forceActiveFocus()
        }

        /*
         Это свойство может помочь избежать некоторых багов отрисовки
         в старых компонентах, заставляя движок использовать
         другой путь для рендеринга.
        */

        // style: TreeViewStyle
        // {
        //     rowDelegate: Rectangle
        //     {
        //         // Этот делегат намеренно не реагирует на наведение (hover),
        //         // чтобы проверить теорию о "залипании" состояния.
        //         // Также он не подсвечивает выделение.
        //         color: "transparent"
        //     }
        // }

        // Колонка 1: Имя (с отступами и иконками дерева)
        TableViewColumn
        {
            role: "label"
            title: "Имя"
            width: 175
        }

        // Колонка 2: Значение
        TableViewColumn
        {
            role: "value"
            title: "Значение"
            width: 100
        }

        // itemDelegate: Item
        // {
        //     ColumnLayout
        //     {

        //     }
        // }

        // // Колонка 2: Значение
        // TableViewColumn
        // {
        //     role: "display"
        //     title: "Значение2"
        //     width: 75
        // }
    }
}
