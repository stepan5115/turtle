grammar TurtleGrammar;

program
    : resize* statement* EOF
    ;

resize
    : 'resize' expression ',' expression ';'?
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
    | assignment                 #AssignmentStmt
    | ifStatement                #IfStmt
    | whileStatement             #WhileStmt
    | forStatement               #ForStmt
    | block                      #BlockStmt
    ;

assignment
    : 'let' IDENTIFIER '=' expression ';'?
    ;

expression
    : '(' expression ')'                         #ParenExpr
    | NUMBER                                     #NumberExpr
    | IDENTIFIER                                 #VariableExpr
    | '-' expression                             #UnaryMinusExpr
    | '!' expression                             #NotExpr
    | expression op=('*'|'/') expression         #MulDivExpr
    | expression op=('+'|'-') expression         #AddSubExpr
    | expression op=('<'|'>'|'<='|'>=') expression #RelationalExpr
    | expression op=('=='|'!=') expression       #EqualityExpr
    | expression '&&' expression                            #AndExpr
    | expression '||' expression                            #OrExpr
    ;

ifStatement
    : 'if' '(' expression ')' statement ('else' statement)?
    ;

whileStatement
    : 'while' '(' expression ')' statement
    ;

forStatement
    : 'for' '(' assignment? ';' expression? ';' assignment? ')' statement
    ;

block
    : '{' statement* '}'
    ;

penUpCommand
    : 'pen_up' ';'?
    ;

penDownCommand
    : 'pen_down' ';'?
    ;

moveCommand
    : 'move' expression ';'?
    ;

moveToCommand
    : 'move_to' expression ',' expression ';'?
    ;

setColorCommand
    : 'color' expression ',' expression ',' expression ';'?
    ;

fillFieldCommand
    : 'fill' expression ',' expression ',' expression ';'?
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

IDENTIFIER
    : [a-zA-Z_][a-zA-Z0-9_]*
    ;

WS
    : [ \t\r\n]+ -> skip
    ;

COMMENT
    : '//' ~[\r\n]* -> skip
    ;
COMMENT_MULTI
    : '/*' .*? '*/' -> skip
    ;