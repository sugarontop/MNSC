#include "pch.h"
#include <iostream>
#include <cassert>
#include "TextContainer.h"

using namespace TSF;

void TestInsertAndGetText() {
    CTextContainer container;
    UINT resultCnt;
    container.InsertText(0, L"Hello", 5, resultCnt);
    assert(container.GetTextLength() == 5);

    WCHAR buffer[10];
    container.GetText(0, buffer, 5);
    assert(wcscmp(buffer, L"Hello") == 0);

    container.InsertText(5, L" World", 6, resultCnt);
    assert(container.GetTextLength() == 11);

    //container.GetText(0, buffer, 11);
    //assert(wcscmp(buffer, L"Hello World") == 0);
}

void TestRemoveText() {
    CTextContainer container;
    UINT resultCnt;
    container.InsertText(0, L"Hello World", 11, resultCnt);
    container.RemoveText(5, 6);
    assert(container.GetTextLength() == 5);

    WCHAR buffer[10];
    container.GetText(0, buffer, 5);
    assert(wcscmp(buffer, L"Hello") == 0);
}

void TestUndo() {
    CTextContainer container;
    UINT resultCnt;
    container.InsertText(0, L"Hello", 5, resultCnt);
    container.InsertText(5, L" World", 6, resultCnt);

    container.Undo();
    WCHAR buffer[10];
    container.GetText(0, buffer, 5);
    assert(wcscmp(buffer, L"Hello") == 0);
}

void TestSelection() {
    CTextContainer container;
    UINT resultCnt;
    container.InsertText(0, L"Hello World", 11, resultCnt);
    container.SetSelStart(0);
    container.SetSelEnd(5);
    std::wstring selection;
    assert(container.GetSelectionText(&selection));
    assert(selection == L"Hello");
}

void TestLineNo() {
    CTextContainer container;
    UINT resultCnt;
    container.InsertText(0, L"Line 1\nLine 2\nLine 3", 20, resultCnt);
    assert(container.LineNo(8) == 1);
    assert(container.LineNo(15) == 2);
}

void TestGetRowText() {
    CTextContainer container;
    UINT resultCnt;
    container.InsertText(0, L"First Line\nSecond Line", 23, resultCnt);
    assert(container.GetRowText(5) == L"First");
    assert(container.GetRowText(18) == L"Second Li");
}


int main() {
    TestInsertAndGetText();
    TestRemoveText();
    TestUndo();
    TestSelection();
    TestLineNo();
    TestGetRowText();

    std::cout << "All tests passed!" << std::endl;

    return 0;
}
