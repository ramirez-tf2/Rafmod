#include "stub/nav.h"


IMPL_DATAMAP(int,                   CFuncNavCost, m_team);
IMPL_DATAMAP(bool,                  CFuncNavCost, m_isDisabled);
IMPL_DATAMAP(string_t,              CFuncNavCost, m_iszTags);
IMPL_REL_AFTER(CUtlVector<CFmtStr>, CFuncNavCost, m_tags, m_iszTags);

MemberFuncThunk<const CFuncNavCost *, bool, const char *> CFuncNavCost::ft_HasTag("CFuncNavCost::HasTag");


#if defined _LINUX

static constexpr uint8_t s_Buf_CNavArea_m_funcNavCostVector[] = {
	0x55,                                                       // +0000  push ebp
	0x89, 0xe5,                                                 // +0001  mov ebp,esp
	0x8b, 0x45, 0x08,                                           // +0003  mov eax,[ebp+this]
	0x81, 0x60, 0x54, 0xff, 0xff, 0xff, 0xdf,                   // +0006  and dword ptr [eax+0x????????],0xdfffffff
	0xc7, 0x80, 0x58, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // +000D  mov dword ptr [eax+0xVVVVVVVV],0x00000000
	0x5d,                                                       // +0017  pop ebp
	0xc3,                                                       // +0018  ret
};

struct CExtract_CNavArea_m_funcNavCostVector : public IExtract<CUtlVector<CHandle<CFuncNavCost>> *>
{
	using T = CUtlVector<CHandle<CFuncNavCost>> *;
	
	CExtract_CNavArea_m_funcNavCostVector() : IExtract<T>(sizeof(s_Buf_CNavArea_m_funcNavCostVector)) {}
	
	virtual bool GetExtractInfo(ByteBuf& buf, ByteBuf& mask) const override
	{
		buf.CopyFrom(s_Buf_CNavArea_m_funcNavCostVector);
		
		mask.SetRange(0x06 + 2, 1, 0x00);
		mask.SetRange(0x0d + 2, 4, 0x00);
		
		return true;
	}
	
	virtual const char *GetFuncName() const override   { return "CNavArea::ClearAllNavCostEntities"; }
	virtual uint32_t GetFuncOffMin() const override    { return 0x0000; }
	virtual uint32_t GetFuncOffMax() const override    { return 0x0000; }
	virtual uint32_t GetExtractOffset() const override { return 0x000d + 2; }
	virtual T AdjustValue(T val) const override        { return reinterpret_cast<T>((uintptr_t)val - 0x0c); }
};

#elif defined _WINDOWS

using CExtract_CNavArea_m_funcNavCostVector = IExtractStub;

#endif


#if defined _LINUX

// CNavArea::Shift
// CNavArea::CheckFloor
// CNavArea::CheckWaterLevel
// CNavArea::ComputeVisibility

static constexpr uint8_t s_Buf_CNavArea_m_center[] = {
	0xf3, 0x0f, 0x10, 0x40, 0x2c, // +0000  movss xmm0,dword ptr [eax+m_center.x]
	0xf3, 0x0f, 0x58, 0x02,       // +0005  addss xmm0,dword ptr [edx.x]
	0xf3, 0x0f, 0x11, 0x40, 0x2c, // +0009  movss dword ptr [eax+m_center.x],xmm0
	0xf3, 0x0f, 0x10, 0x40, 0x30, // +000E  movss xmm0,dword ptr [eax+m_center.y]
	0xf3, 0x0f, 0x58, 0x42, 0x04, // +0013  addss xmm0,dword ptr [edx.y]
	0xf3, 0x0f, 0x11, 0x40, 0x30, // +0018  movss dword ptr [eax+m_center.y],xmm0
	0xf3, 0x0f, 0x10, 0x40, 0x34, // +001D  movss xmm0,dword ptr [eax+m_center.z]
	0xf3, 0x0f, 0x58, 0x42, 0x08, // +0022  addss xmm0,dword ptr [edx.z]
	0xf3, 0x0f, 0x11, 0x40, 0x34, // +0027  movss dword ptr [eax+m_center.z],xmm0
	0x5d,                         // +002C  pop ebp
	0xc3,                         // +002D  ret
};

struct CExtract_CNavArea_m_center : public IExtract<Vector *>
{
	using T = Vector *;
	
	CExtract_CNavArea_m_center() : IExtract<T>(sizeof(s_Buf_CNavArea_m_center)) {}
	
	virtual bool GetExtractInfo(ByteBuf& buf, ByteBuf& mask) const override
	{
		buf.CopyFrom(s_Buf_CNavArea_m_center);
		
		mask.SetRange(0x00 + 4, 1, 0x00);
		mask.SetRange(0x09 + 4, 1, 0x00);
		mask.SetRange(0x0e + 4, 1, 0x00);
		mask.SetRange(0x18 + 4, 1, 0x00);
		mask.SetRange(0x1d + 4, 1, 0x00);
		mask.SetRange(0x27 + 4, 1, 0x00);
		
		return true;
	}
	
	virtual const char *GetFuncName() const override   { return "CNavArea::Shift"; }
	virtual uint32_t GetFuncOffMin() const override    { return 0x0061; }
	virtual uint32_t GetFuncOffMax() const override    { return 0x0061; }
	virtual uint32_t GetExtractOffset() const override { return 0x0000 + 4; }
	virtual T AdjustValue(T val) const override        { return reinterpret_cast<T>((uintptr_t)val & 0x000000ff); }
};

#elif defined _WINDOWS

using CExtract_CNavArea_m_center = IExtractStub;

#endif


#if defined _LINUX

static constexpr uint8_t s_Buf_CNavArea_m_attributeFlags[] = {
	0x55,                                                       // +0000  push ebp
	0x89, 0xe5,                                                 // +0001  mov ebp,esp
	0x8b, 0x45, 0x08,                                           // +0003  mov eax,[ebp+this]
	0x81, 0x60, 0x54, 0xff, 0xff, 0xff, 0xdf,                   // +0006  and dword ptr [eax+0xVVVVVVVV],0xdfffffff
	0xc7, 0x80, 0x58, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // +000D  mov dword ptr [eax+0x????????],0x00000000
	0x5d,                                                       // +0017  pop ebp
	0xc3,                                                       // +0018  ret
};

struct CExtract_CNavArea_m_attributeFlags : public IExtract<int *>
{
	using T = int *;
	
	CExtract_CNavArea_m_attributeFlags() : IExtract<T>(sizeof(s_Buf_CNavArea_m_attributeFlags)) {}
	
	virtual bool GetExtractInfo(ByteBuf& buf, ByteBuf& mask) const override
	{
		buf.CopyFrom(s_Buf_CNavArea_m_attributeFlags);
		
		mask.SetRange(0x06 + 2, 1, 0x00);
		mask.SetRange(0x0d + 2, 4, 0x00);
		
		return true;
	}
	
	virtual const char *GetFuncName() const override   { return "CNavArea::ClearAllNavCostEntities"; }
	virtual uint32_t GetFuncOffMin() const override    { return 0x0000; }
	virtual uint32_t GetFuncOffMax() const override    { return 0x0000; }
	virtual uint32_t GetExtractOffset() const override { return 0x0006 + 2; }
	virtual T AdjustValue(T val) const override        { return reinterpret_cast<T>((uintptr_t)val & 0x000000ff); }
};

#elif defined _WINDOWS

using CExtract_CNavArea_m_attributeFlags = IExtractStub;

#endif


#if defined _LINUX

static constexpr uint8_t s_Buf_CNavArea_m_costSoFar[] = {
	0xf3, 0x0f, 0x59, 0xd2,       // +0000  mulss xmm2,xmm2
	0xf3, 0x0f, 0x59, 0xc9,       // +0004  mulss xmm1,xmm1
	0xf3, 0x0f, 0x58, 0xc2,       // +0008  addss xmm0,xmm2
	0xf3, 0x0f, 0x58, 0xc1,       // +000C  addss xmm0,xmm1
	0xf3, 0x0f, 0x51, 0xc0,       // +0010  sqrtss xmm0,xmm0
	0xf3, 0x0f, 0x58, 0x42, 0x44, // +0014  addss xmm0,dword ptr [edx+0xVVVVVVVV]
	0xf3, 0x0f, 0x11, 0x46, 0x44, // +0019  movss dword ptr [ebx+0xVVVVVVVV],xmm0
};

struct CExtract_CNavArea_m_costSoFar : public IExtract<float *>
{
	using T = float *;
	
	CExtract_CNavArea_m_costSoFar() : IExtract<T>(sizeof(s_Buf_CNavArea_m_costSoFar)) {}
	
	virtual bool GetExtractInfo(ByteBuf& buf, ByteBuf& mask) const override
	{
		buf.CopyFrom(s_Buf_CNavArea_m_costSoFar);
		
		mask.SetRange(0x14 + 4, 1, 0x00);
		mask.SetRange(0x19 + 4, 1, 0x00);
		
		return true;
	}
	
	virtual const char *GetFuncName() const override   { return "ISearchSurroundingAreasFunctor::IterateAdjacentAreas"; }
	virtual uint32_t GetFuncOffMin() const override    { return 0x00a0; } // @ 0x00ea
	virtual uint32_t GetFuncOffMax() const override    { return 0x0100; }
	virtual uint32_t GetExtractOffset() const override { return 0x0014 + 4; }
	virtual T AdjustValue(T val) const override        { return reinterpret_cast<T>((uintptr_t)val & 0x000000ff); }
};

#elif defined _WINDOWS

using CExtract_CNavArea_m_costSoFar = IExtractStub;

#endif


#if defined _LINUX

// CTFNavArea::m_nAttributes
// * CTFNavArea::IsValidForWanderingPopulation
// * CTFNavArea::IsBlocked
// * CTFNavMesh::IsSentryGunHere
//   CTFNavMesh::ResetMeshAttributes
//   CTFNavMesh::RemoveAllMeshDecoration
//   CTFNavMesh::ComputeLegalBombDropAreas
//   CTFNavMesh::CollectAndMaskSpawnRoomExits
//   TF_EditClearAllAttributes
//   GetBombInfo

static constexpr uint8_t s_Buf_CTFNavArea_m_nAttributes[] = {
	0x55,                                                       // +0000  push ebp
	0x89, 0xe5,                                                 // +0001  mov ebp,esp
	0x8b, 0x45, 0x08,                                           // +0003  mov eax,[ebp+this]
	0x5d,                                                       // +0006  pop ebp
	0xf7, 0x80, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x06, // +0007  test DWORD PTR [eax+0xVVVVVVVV],0x06000007
	0x0f, 0x94, 0xc0,                                           // +0011  setz al
	0xc3,                                                       // +0014  ret
};

struct CExtract_CTFNavArea_m_nAttributes : public IExtract<TFNavAttributeType *>
{
	CExtract_CTFNavArea_m_nAttributes() : IExtract<TFNavAttributeType *>(sizeof(s_Buf_CTFNavArea_m_nAttributes)) {}
	
	virtual bool GetExtractInfo(ByteBuf& buf, ByteBuf& mask) const override
	{
		buf.CopyFrom(s_Buf_CTFNavArea_m_nAttributes);
		
		mask.SetRange(0x07 + 2, 4, 0x00);
		
		return true;
	}
	
	virtual const char *GetFuncName() const override   { return "CTFNavArea::IsValidForWanderingPopulation"; }
	virtual uint32_t GetFuncOffMin() const override    { return 0x0000; }
	virtual uint32_t GetFuncOffMax() const override    { return 0x0030; }
	virtual uint32_t GetExtractOffset() const override { return 0x0007 + 2; }
};

#elif defined _WINDOWS

static constexpr uint8_t s_Buf_CTFNavArea_m_nAttributes[] = {
	0x55,                               // +0000  push ebp
	0x8b, 0xec,                         // +0001  mov ebp,esp
	0x8b, 0x91, 0x00, 0x00, 0x00, 0x00, // +0003  mov edx,[ecx+0xVVVVVVVV]
};

struct CExtract_CTFNavArea_m_nAttributes : public IExtract<TFNavAttributeType *>
{
	CExtract_CTFNavArea_m_nAttributes() : IExtract<TFNavAttributeType *>(sizeof(s_Buf_CTFNavArea_m_nAttributes)) {}
	
	virtual bool GetExtractInfo(ByteBuf& buf, ByteBuf& mask) const override
	{
		buf.CopyFrom(s_Buf_CTFNavArea_m_nAttributes);
		
		mask.SetRange(0x03 + 2, 4, 0x00);
		
		return true;
	}
	
	virtual const char *GetFuncName() const override   { return "CTFNavArea::IsBlocked"; }
	virtual uint32_t GetFuncOffMin() const override    { return 0x0000; }
	virtual uint32_t GetFuncOffMax() const override    { return 0x0030; }
	virtual uint32_t GetExtractOffset() const override { return 0x0003 + 2; }
};

#endif


#if defined _LINUX

static constexpr uint8_t s_Buf_CTFNavArea_m_IncursionDistances[] = {
	0x8b, 0x45, 0x08,                                     // +0000  mov eax,[ebp+this]
	0x8b, 0x7d, 0x0c,                                     // +0003  mov edi,[ebp+arg_4]
	0xf3, 0x0f, 0x10, 0x84, 0xb8, 0x60, 0x01, 0x00, 0x00, // +0006  movss xmm0,dword ptr [eax+edi*4+0xVVVVVVVV]
};

struct CExtract_CTFNavArea_m_IncursionDistances : public IExtract<float (*)[4]>
{
	CExtract_CTFNavArea_m_IncursionDistances() : IExtract<float (*)[4]>(sizeof(s_Buf_CTFNavArea_m_IncursionDistances)) {}
	
	virtual bool GetExtractInfo(ByteBuf& buf, ByteBuf& mask) const override
	{
		buf.CopyFrom(s_Buf_CTFNavArea_m_IncursionDistances);
		
		mask.SetRange(0x06 + 5, 4, 0x00);
		
		return true;
	}
	
	virtual const char *GetFuncName() const override   { return "CTFNavArea::GetNextIncursionArea"; }
	virtual uint32_t GetFuncOffMin() const override    { return 0x0000; }
	virtual uint32_t GetFuncOffMax() const override    { return 0x0030; }
	virtual uint32_t GetExtractOffset() const override { return 0x0006 + 5; }
};

#elif defined _WINDOWS

using CExtract_CTFNavArea_m_IncursionDistances = IExtractStub;

#endif


IMPL_EXTRACT  (CUtlVector<CHandle<CFuncNavCost>>, CNavArea, m_funcNavCostVector, new CExtract_CNavArea_m_funcNavCostVector());
IMPL_EXTRACT  (Vector,                            CNavArea, m_center,            new CExtract_CNavArea_m_center());
IMPL_EXTRACT  (int,                               CNavArea, m_attributeFlags,    new CExtract_CNavArea_m_attributeFlags());
IMPL_RELATIVE (CNavArea *,                        CNavArea, m_parent,            m_attributeFlags, (116 - 80));
IMPL_REL_AFTER(int,                               CNavArea, m_parentHow,         m_parent);
IMPL_EXTRACT  (float,                             CNavArea, m_costSoFar,         new CExtract_CNavArea_m_costSoFar());

MemberFuncThunk <const CNavArea *, void, Extent *>                               CNavArea::ft_GetExtent                            ("CNavArea::GetExtent");
MemberFuncThunk <const CNavArea *, void, const Vector *, Vector *>               CNavArea::ft_GetClosestPointOnArea                ("CNavArea::GetClosestPointOnArea");
MemberFuncThunk <const CNavArea *, float, const CNavArea *>                      CNavArea::ft_ComputeAdjacentConnectionHeightChange("CNavArea::ComputeAdjacentConnectionHeightChange");
MemberFuncThunk <const CNavArea *, float, float, float>                          CNavArea::ft_GetZ                                 ("CNavArea::GetZ [float float]");
MemberVFuncThunk<const CNavArea *, void, int, int, int, int, float, bool, float> CNavArea::vt_DrawFilled                           (TypeName<CNavArea>(), "CNavArea::DrawFilled");


IMPL_EXTRACT(TFNavAttributeType, CTFNavArea, m_nAttributes,        new CExtract_CTFNavArea_m_nAttributes());
IMPL_EXTRACT(float[4],           CTFNavArea, m_IncursionDistances, new CExtract_CTFNavArea_m_IncursionDistances());

MemberFuncThunk<const CTFNavArea *, bool, int, bool> CTFNavArea::ft_IsBlocked         ("CTFNavArea::IsBlocked");
MemberFuncThunk<const CTFNavArea *, float>           CTFNavArea::ft_GetCombatIntensity("CTFNavArea::GetCombatIntensity");


MemberFuncThunk<const CNavMesh *, CNavArea *, const Vector&, float>                        CNavMesh::ft_GetNavArea_vec                          ("CNavMesh::GetNavArea [vec]");
MemberFuncThunk<const CNavMesh *, CNavArea *, CBaseEntity *, int, float>                   CNavMesh::ft_GetNavArea_ent                          ("CNavMesh::GetNavArea [ent]");
MemberFuncThunk<const CNavMesh *, CNavArea *, const Vector&, bool, float, bool, bool, int> CNavMesh::ft_GetNearestNavArea_vec                   ("CNavMesh::GetNearestNavArea [vec]");
MemberFuncThunk<const CNavMesh *, CNavArea *, CBaseEntity *, int, float>                   CNavMesh::ft_GetNearestNavArea_ent                   ("CNavMesh::GetNearestNavArea [ent]");
MemberFuncThunk<const CNavMesh *, bool, const Vector&, float *, Vector *>                  CNavMesh::ft_GetGroundHeight                         ("CNavMesh::GetGroundHeight");
MemberFuncThunk<CNavMesh *, void, const Extent&, CUtlVector<CTFNavArea *> *>               CNavMesh::ft_CollectAreasOverlappingExtent_CTFNavArea("CNavMesh::CollectAreasOverlappingExtent<CTFNavArea>");


MemberFuncThunk<CTFNavMesh *, void, CUtlVector<CBaseObject *> *, int> CTFNavMesh::ft_CollectBuiltObjects("CTFNavMesh::CollectBuiltObjects");


GlobalThunk<CTFNavMesh *>             TheNavMesh ("TheNavMesh");
GlobalThunk<CUtlVector<CTFNavArea *>> TheNavAreas("TheNavAreas");


StaticFuncThunk<float, CNavArea *, CNavArea *, CTFBotPathCost&, float>                                        ft_NavAreaTravelDistance_CTFBotPathCost("NavAreaTravelDistance<CTFBotPathCost>");
StaticFuncThunk<bool, CNavArea *, CNavArea *, const Vector *, CTFBotPathCost&, CNavArea **, float, int, bool> ft_NavAreaBuildPath_CTFBotPathCost     ("NavAreaBuildPath<CTFBotPathCost>");
