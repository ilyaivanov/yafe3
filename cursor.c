#pragma once 

#include "types.h"
#include "string.c"

typedef struct Cursor
{
    i32 pos;
    i32 desiredCol;
} Cursor;

// modes
// don't jump lines (postponed)
// desired col

void CursorMoveLeft(Cursor* cursor, StringBuffer* buffer)
{
    if(cursor->pos > 0)
    {
        cursor->pos--;
        // cursor->desiredCol = cursor->pos;
    }
}

void CursorMoveRight(Cursor* cursor, StringBuffer* buffer)
{
    if(cursor->pos < buffer->size - 1)
    {
        cursor->pos++;
        // cursor->desiredCol = cursor->pos;
    }
}



void CursorMoveDown(Cursor* cursor, StringBuffer* buffer)
{
    i32 index = IndexBefore(buffer, cursor->pos, '\n');
    i32 lineStart = index == -1 ? 0 : index + 1;
    i32 lineOffset = cursor->pos - lineStart + 1;

    i32 nextLine = IndexAfter(buffer, cursor->pos - 1, '\n');
    if(nextLine != -1)
    {
        i32 target = nextLine + lineOffset;
        i32 lineAfterNextLine = IndexAfter(buffer, nextLine, '\n');
        if(target > lineAfterNextLine)
            cursor->pos = lineAfterNextLine;
        else 
            cursor->pos = nextLine + lineOffset;
    }
}

void CursorMoveUp(Cursor* cursor, StringBuffer* buffer)
{
    i32 index = IndexBefore(buffer, cursor->pos, '\n');
    i32 lineStart = index == -1 ? 0 : index + 1;
    i32 lineOffset = cursor->pos - lineStart + 1;

    if(index != -1)
    {
        i32 prevLine = IndexBefore(buffer, index, '\n');
        if(prevLine != -1)
            cursor->pos = prevLine + lineOffset;
        else 
            cursor->pos = lineOffset - 1;
    }
}
