
// HikCamDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "HikCam.h"
#include "HikCamDlg.h"
#include "afxdialogex.h"
#include<opencv2/opencv.hpp>
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
using namespace cv;
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHikCamDlg 对话框



CHikCamDlg::CHikCamDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HIKCAM_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHikCamDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, res_list);
}

BEGIN_MESSAGE_MAP(CHikCamDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CHikCamDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CHikCamDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CHikCamDlg::OnBnClickedButton3)
	ON_BN_CLICKED(IDC_BUTTON4, &CHikCamDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CHikCamDlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CHikCamDlg 消息处理程序

BOOL CHikCamDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHikCamDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHikCamDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CHikCamDlg::PrintMessage(const char* pszFormat, ...)
{
	va_list args;
	va_start(args, pszFormat);
	char   szInfo[512] = { 0 };
	vsprintf_s(szInfo, 512, pszFormat, args);
	va_end(args);
	res_list.AddString(CA2T(szInfo));
	res_list.SetTopIndex(res_list.GetCount() - 1);
}

void CHikCamDlg::OnBnClickedButton1()
{
	// TODO: 在此添加控件通知处理程序代码
	int nRet = CMvCamera::EnumDevices(MV_GIGE_DEVICE | MV_USB_DEVICE, &m_stDevList);
	if (MV_OK != nRet || m_stDevList.nDeviceNum == 0)
	{
		PrintMessage("Find no device!\r\n");
		return;
	}
	PrintMessage("Find %d devices!\r\n", m_stDevList.nDeviceNum);
;
	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{
		char chDeviceName[256] = { 0 };
		MV_CC_DEVICE_INFO* pDeviceInfo = m_stDevList.pDeviceInfo[i];
		if (pDeviceInfo->nTLayerType == MV_USB_DEVICE)
		{
			if (strcmp("", (LPCSTR)(pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName)) != 0)
			{
				sprintf_s(chDeviceName, 256, "%s [%s]", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chUserDefinedName);
			}
			else
			{
				sprintf_s(chDeviceName, 256, "%s (%s)", pDeviceInfo->SpecialInfo.stUsb3VInfo.chModelName, pDeviceInfo->SpecialInfo.stUsb3VInfo.chSerialNumber);
			}
		}
		PrintMessage(chDeviceName);
		//wchar_t strUserName[128] = { 0 };
		//MultiByteToWideChar(CP_ACP, 0, (LPCSTR)(chDeviceName), -1, strUserName, 128);
		
	}
	UpdateData(FALSE);
}
// ch:像素排列由RGB转为BGR | en:Convert pixel arrangement from RGB to BGR
void RGB2BGR(unsigned char* pRgbData, unsigned int nWidth, unsigned int nHeight)
{
	if (NULL == pRgbData)
	{
		return;
	}

	// red和blue数据互换
	for (unsigned int j = 0; j < nHeight; j++)
	{
		for (unsigned int i = 0; i < nWidth; i++)
		{
			unsigned char red = pRgbData[j * (nWidth * 3) + i * 3];
			pRgbData[j * (nWidth * 3) + i * 3] = pRgbData[j * (nWidth * 3) + i * 3 + 2];
			pRgbData[j * (nWidth * 3) + i * 3 + 2] = red;
		}
	}
}

// ch:帧数据转换为Mat格式图片并保存 | en:Convert data stream to Mat format then save image
bool Convert2Mat(MV_FRAME_OUT_INFO_EX* pstImageInfo, unsigned char* pData,Mat &srcImage)
{
	if (NULL == pstImageInfo || NULL == pData)
	{
		printf("NULL info or data.\n");
		return false;
	}
	if (PixelType_Gvsp_RGB8_Packed == pstImageInfo->enPixelType)     // RGB8类型
	{
		// Mat像素排列格式为BGR，需要转换
		RGB2BGR(pData, pstImageInfo->nWidth, pstImageInfo->nHeight);
		srcImage = cv::Mat(pstImageInfo->nHeight, pstImageInfo->nWidth, CV_8UC3, pData);
	}
	return true;
}
void draw_mat(Mat m_cvImg, CWnd* cwnd, UINT ID)
{
	cv::Mat img;
	CRect rect;

	cwnd->GetDlgItem(ID)->GetClientRect(&rect);
	if (rect.Width() % 4 != 0)
	{
		rect.SetRect(rect.left, rect.top, rect.left + (rect.Width() + 3) / 4 * 4, rect.bottom);  //调整图像宽度为4的倍数
		cwnd->GetDlgItem(ID)->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOMOVE);
	}

	cv::Rect dst(rect.left, rect.top, rect.right, rect.bottom);
	cv::resize(m_cvImg, img, cv::Size(rect.Width(), rect.Height()));  //使图像适应控件大小

	unsigned int m_buffer[sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 256];
	BITMAPINFO* m_bmi = (BITMAPINFO*)m_buffer;
	BITMAPINFOHEADER* m_bmih = &(m_bmi->bmiHeader);
	memset(m_bmih, 0, sizeof(*m_bmih));
	m_bmih->biSize = sizeof(BITMAPINFOHEADER);
	m_bmih->biWidth = img.cols;   //必须为4的倍数
	m_bmih->biHeight = -img.rows; //在自下而上的位图中 高度为负
	m_bmih->biPlanes = 1;
	m_bmih->biCompression = BI_RGB;
	m_bmih->biBitCount = 8 * img.channels();

	if (img.channels() == 1)  //当图像为灰度图像时需要设置调色板颜色
	{
		for (int i = 0; i < 256; i++)
		{
			m_bmi->bmiColors[i].rgbBlue = i;
			m_bmi->bmiColors[i].rgbGreen = i;
			m_bmi->bmiColors[i].rgbRed = i;
			m_bmi->bmiColors[i].rgbReserved = 0;
		}
	}
	CDC* pDC = cwnd->GetDlgItem(ID)->GetDC();
	::StretchDIBits(pDC->GetSafeHdc(), 0, 0, rect.Width(), rect.Height(), 0, 0, rect.Width(), rect.Height(), img.data, (BITMAPINFO*)m_bmi, DIB_RGB_COLORS, SRCCOPY);
	cwnd->ReleaseDC(pDC);
}
CMvCamera* m_pcMyCamera[2] = {};
struct Para{
	CWnd* inst;
	int pic;
};
Para para[2] = {};
void __stdcall ImageCallBackEx(unsigned char* pData, MV_FRAME_OUT_INFO_EX* pFrameInfo, void* pUser)
{
	Para* pa = (Para*)pUser;
	if (pFrameInfo)
	{
		printf("Get One Frame: Width[%d], Height[%d], nFrameNum[%d]\n",
			pFrameInfo->nWidth, pFrameInfo->nHeight, pFrameInfo->nFrameNum);
		//first set cam image to bgr8 format.
		Mat mat = cv::Mat(pFrameInfo->nHeight, pFrameInfo->nWidth, CV_8UC3, pData);
		Mat mp;
		resize(mat, mp, Size(400, 300));
		draw_mat(mp, pa->inst, pa->pic);
	}
}

void CHikCamDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	para[0].inst = this;
	para[0].pic = IDC_STATIC1;
	para[1].inst = this;
	para[1].pic = IDC_STATIC2;

	for (unsigned int i = 0; i < m_stDevList.nDeviceNum; i++)
	{
		if (NULL == m_pcMyCamera[i])
		{
			m_pcMyCamera[i] = new CMvCamera;
		}
		int nRet = m_pcMyCamera[i]->Open(m_stDevList.pDeviceInfo[i]);
		if (MV_OK != nRet)
		{
			delete(m_pcMyCamera[i]);
			m_pcMyCamera[i] = NULL;
			PrintMessage("Open device failed! DevIndex[%d], nRet[%#x]\r\n", i + 1, nRet);
			continue;
		}
		else
		{
			PrintMessage("Open device %d",i);
		}
		nRet = m_pcMyCamera[i]->SetEnumValue("TriggerMode", MV_TRIGGER_MODE_ON);
		if (MV_OK != nRet)
		{
			PrintMessage("Set Trigger source fail!");
		}
		nRet = m_pcMyCamera[i]->SetEnumValue("TriggerSource", MV_TRIGGER_SOURCE_SOFTWARE);
		if (MV_OK != nRet)
		{
			PrintMessage("Set Trigger source fail!");
		}
		nRet = m_pcMyCamera[i]->SetFloatValue("ExposureTime", 200000);
		if (MV_OK != nRet)
		{
			PrintMessage("Set exposure fail!");
		}
		nRet = m_pcMyCamera[i]->SetFloatValue("Gain", 0);
		if (MV_OK != nRet)
		{
			PrintMessage("Set gain fail!");
		}
		m_pcMyCamera[i]->RegisterImageCallBack(ImageCallBackEx, &para[i]);
		m_pcMyCamera[i]->StartGrabbing();
	}
}


void CHikCamDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	ExitProcess(0);
}


void CHikCamDlg::OnBnClickedButton4()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pcMyCamera[1]->CommandExecute("TriggerSoftware");
}


void CHikCamDlg::OnBnClickedButton5()
{
	// TODO: 在此添加控件通知处理程序代码
	m_pcMyCamera[0]->CommandExecute("TriggerSoftware");
}
