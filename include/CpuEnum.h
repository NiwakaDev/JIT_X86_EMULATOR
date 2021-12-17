#pragma once
enum REGISTER_KIND {
    EAX, 
    ECX, 
    EDX, 
    EBX, 
    ESP, 
    EBP, 
    ESI, 
    EDI, 
    REGISTER_KIND_TOTAL,
    AL = EAX,
    CL = ECX,
    DL = EDX,
    BL = EBX,
    AH = AL+4,
    CH = CL+4,
    DH = DL+4,
    BH = BL+4,
};


