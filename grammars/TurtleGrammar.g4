grammar TurtleGrammar;

program
    : statement* EOF
    ;

statement
    : penUpCommand               #PenUpStmt
    | penDownCommand             #PenDownStmt
    | moveCommand                #MoveStmt
    | moveToCommand              #MoveToStmt
    | setColorCommand            #SetColorStmt
    | fillFieldCommand           #FillFieldStmt
    | setDrawModeCommand         #SetDrawModeStmt
    | orientationCommand         #OrientationStmt
    ;

penUpCommand
    : 'pen_up' ';'?
    ;

penDownCommand
    : 'pen_down' ';'?
    ;

moveCommand
    : 'move' NUMBER ';'?
    ;

moveToCommand
    : 'move_to' NUMBER ',' NUMBER ';'?
    ;

setColorCommand
    : 'color' NUMBER ',' NUMBER ',' NUMBER ';'?
    ;

fillFieldCommand
    : 'fill' NUMBER ',' NUMBER ',' NUMBER ';'?
    ;

setDrawModeCommand
    : 'draw_mode' boolean ';'?
    ;

orientationCommand
    : direction ';'?
    ;

direction
    : 'right'
    | 'left'
    | 'up'
    | 'down'
    ;

boolean
    : 'true'
    | 'false'
    ;

NUMBER
    : [0-9]+
    ;

WS
    : [ \t\r\n]+ -> skip
    ;

COMMENT
    : '//' ~[\r\n]* -> skip
    ;