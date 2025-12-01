// Problem8View.h : CProblem8View 클래스의 인터페이스
//

#pragma once
#include <atlimage.h>   // CImage
#include <vector>
#include <cmath>

class CProblem8Doc;

class CProblem8View : public CView
{
protected:
    CProblem8View() noexcept;
    DECLARE_DYNCREATE(CProblem8View)

public:
    CProblem8Doc* GetDocument() const;

    virtual void OnInitialUpdate();
    virtual void OnDraw(CDC* pDC);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

public:
    virtual ~CProblem8View();

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
#endif

protected:
    DECLARE_MESSAGE_MAP()

public:
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);

    // --- 배경 이미지(CImage) ---
    CImage m_bg;

    // --- 그래프 구조체 ---
    struct Node
    {
        CPoint pos;
        bool   selected = false;
        bool   inPath = false;
    };

    struct Edge
    {
        int    from;
        int    to;
        double weight;
        bool   inPath = false;
    };

    std::vector<Node> m_nodes;
    std::vector<Edge> m_edges;

    int m_start = -1;
    int m_end = -1;

    int    FindNode(CPoint p);
    void   AddEdge(int a, int b);
    void   ClearPath();
    void   RunDijkstra(int start, int goal);
    double Distance(CPoint a, CPoint b);
};

#ifndef _DEBUG
inline CProblem8Doc* CProblem8View::GetDocument() const
{
    return reinterpret_cast<CProblem8Doc*>(m_pDocument);
}
#endif
