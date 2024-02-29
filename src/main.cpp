// Dear ImGui: standalone example application for DirectX 10

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui/imgui.h"
#include "imgui/imgui_impl_win32.h"
#include "imgui/imgui_impl_dx10.h"
#include <d3d10_1.h>
#include <d3d10.h>
#include <tchar.h>
#include <cstdio>

// Data
static ID3D10Device*            g_pd3dDevice = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D10RenderTargetView*  g_mainRenderTargetView = nullptr;


// Helper functions

void CreateRenderTarget();
void CleanupRenderTarget();


bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
    HRESULT res = D3D10CreateDeviceAndSwapChain(nullptr, D3D10_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D10CreateDeviceAndSwapChain(nullptr, D3D10_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, D3D10_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D10Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}




#include <unistd.h>
#include <pthread.h>
#include <winsock.h>
#include "wave.h"



static pthread_mutex_t g_mutex;

static SceWaveParam g_wave_param;

static int g_alive = 1;
static bool g_auto_update = false;
static bool g_exec_update = false;
static bool g_exec_load = false;

static bool s_save_dialog_to_pc = false;
static bool s_save_dialog_to_vita = false;
static bool s_exec_save_to_vita = false;
static char save_name[128] = "waveparam.bin";

#include <libusb.h>
#include "psp2wpp.h"

static psp2wpp_comm_packet g_comm_packet;
static uint32_t sequence = 0;

int _h0(uint32_t cmd){

	int res;

	res = psp2wpp_usb_recv_ex(&g_comm_packet, sizeof(g_comm_packet), 5000);
	if(res < 0){
		printf("%s: psp2wpp_usb_recv_ex 0x%X\n", __FUNCTION__, res);
		return res;
	}

	uint32_t recv_cmd = g_comm_packet.cmd;

	if(recv_cmd != 0){
		if(g_comm_packet.sequence != sequence){
			printf("%s: Bad sequence\n", __FUNCTION__);
			sequence = 0;
			return -2;
		}

		sequence += 1;
	}

	g_comm_packet.sequence = sequence;
	g_comm_packet.cmd      = cmd;
	g_comm_packet.result   = recv_cmd == 0;
	g_comm_packet.size     = 0;

	res = psp2wpp_usb_send_ex(&g_comm_packet, sizeof(g_comm_packet), 5000);
	if(res < 0){
		printf("%s: psp2wpp_usb_send_ex 0x%X\n", __FUNCTION__, res);
		return res;
	}

	return g_comm_packet.result;
}

void *usb_comm_thread(void *argp){

	int alive = 1;
	SceWaveParam wave_param, prev_wave_param;

	libusb_init(NULL);
	libusb_set_debug(NULL, LIBUSB_LOG_LEVEL_WARNING);

	usleep(160 * 1000);

	memset(&wave_param, 0, sizeof(wave_param));
	memset(&prev_wave_param, 0, sizeof(prev_wave_param));

	int res, resp;

	while(alive){

		pthread_mutex_lock(&g_mutex);
		memcpy(&wave_param, &g_wave_param, sizeof(wave_param));
		alive = g_alive;
		pthread_mutex_unlock(&g_mutex);

		if(alive == 0){
			break;
		}

		if(psp2wpp_is_connected() == 0){
			res = psp2wpp_open_ex_2();
			if(res < 0){
				sleep(1);
				continue;
			}
		}

		printf("Connected\n");

		sequence = 0;

		while(alive){
			pthread_mutex_lock(&g_mutex);
			memcpy(&wave_param, &g_wave_param, sizeof(wave_param));
			alive = g_alive;
			pthread_mutex_unlock(&g_mutex);

			if(alive == 0){
				break;
			}

			res = _h0(0);
			if(res < 0){
				printf("%s: _h0 0x%X\n", __FUNCTION__, res);
				break;
			}

			if(s_exec_save_to_vita == true){
				s_exec_save_to_vita = false;
				s_save_dialog_to_vita = false;

				res = _h0(3);
				if(res < 0){
					break;
				}

				psp2wpp_usb_send_ex(&wave_param, sizeof(wave_param), 1000);
				psp2wpp_usb_recv_ex(&resp, sizeof(resp), 5000);

				if(resp == 0){
					printf("saved waveparam!\n");
				}else{
					printf("failed save waveparam (0x%X)\n", resp);
				}
			}

			if(g_exec_update == true || (g_auto_update == true && memcmp(&prev_wave_param, &wave_param, sizeof(wave_param)) != 0)){
				g_exec_update = false;

				wave_param.color_index = 0x1F;

				res = _h0(1);
				if(res < 0){
					break;
				}

				psp2wpp_usb_send_ex(&wave_param, sizeof(wave_param), 5000);
				psp2wpp_usb_recv_ex(&res, sizeof(res), 5000);

				memcpy(&prev_wave_param, &wave_param, sizeof(prev_wave_param));
			}

			if(g_exec_load != false){
				g_exec_load = false;

				int res, resp;

				res = _h0(2);
				if(res < 0){
					break;
				}

				res = psp2wpp_usb_recv_ex(&resp, sizeof(resp), 5000);
				if(res < 0){
					printf("psp2wpp_usb_recv_ex 0x%X\n", res);
					break;
				}

				if(resp < 0){
					printf("failed load current wave (0x%X)\n", resp);
					continue;
				}

				res = psp2wpp_usb_recv_ex(&wave_param, sizeof(wave_param), 5000);
				if(res < 0){
					printf("psp2wpp_usb_recv_ex 0x%X\n", res);
					break;
				}

				memcpy(&prev_wave_param, &wave_param, sizeof(prev_wave_param));

				printf("Load current wave!\n");

				pthread_mutex_lock(&g_mutex);
				memcpy(&g_wave_param, &wave_param, sizeof(g_wave_param));
				pthread_mutex_unlock(&g_mutex);
			}

			usleep(16 * 1000);
		}
		printf("Disconnected\n");

		psp2wpp_close_ex();

/*

		if(current_color_index != g_current_color_index){
			current_color_index = g_current_color_index;
			// send_wave_index(current_color_index);
		}

*/

		// usleep(1 * 1000 * 1000);
		usleep(16 * 1000);
	}

	psp2wpp_close_ex();
	libusb_exit(NULL);

	return NULL;
}

int psp2wpp_settings_draw_main_settings(void){

	ImGui::SliderInt("color_index", (int *)&(g_wave_param.color_index), 0, 0x1F);
	if(ImGui::IsItemHovered()){
		ImGui::SetTooltip("Fixed to 31 when used in \"waveparam.bin\".");
	}

	ImGui::ColorEdit4("selecter 0", (float *)&(g_wave_param.selecter[0]));
	ImGui::ColorEdit4("selecter 1", (float *)&(g_wave_param.selecter[1]));

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("Material")){
		ImGui::ColorEdit4("color",              (float *)&(g_wave_param.Material.color));
		ImGui::SliderFloat("diffuse_coeff",     &(g_wave_param.Material.diffuse_coeff),     0.0f, 1.0f);
		ImGui::SliderFloat("specular_coeff",    &(g_wave_param.Material.specular_coeff),    0.0f, 3.0f);
		ImGui::SliderFloat("specular_power",    &(g_wave_param.Material.specular_power),    0.0f, 100.0f);
		ImGui::SliderFloat("fresnel_front",     &(g_wave_param.Material.fresnel_front),     0.0f, 1.0f);
		ImGui::SliderFloat("fresnel_control",   &(g_wave_param.Material.fresnel_control),   0.0f, 1.0f);
		ImGui::SliderFloat("fresnel_coeff",     &(g_wave_param.Material.fresnel_coeff),     0.0f, 1.0f);
		ImGui::SliderFloat("refraction_coeff",  &(g_wave_param.Material.refraction_coeff),  0.0f, 3.0f);
		ImGui::SliderFloat("refraction_amount", &(g_wave_param.Material.refraction_amount), 0.0f, 1.0f);
		ImGui::SliderFloat("second_reflection", &(g_wave_param.Material.second_reflection), 0.0f, 1.0f);
		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("PointLightSphere")){

		ImGui::SliderFloat("theta_phi 0", &(g_wave_param.PointLightSphere.theta_phi[0]), 0.0f, 5.0f);
		ImGui::SliderFloat("theta_phi 1", &(g_wave_param.PointLightSphere.theta_phi[1]), -5.0f, 5.0f);

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("attn")){
			ImGui::SliderFloat("a", &(g_wave_param.PointLightSphere.attn.a), 0.0f, 1.0f);
			ImGui::SliderFloat("b", &(g_wave_param.PointLightSphere.attn.b), 0.0f, 1.0f);
			ImGui::SliderFloat("c", &(g_wave_param.PointLightSphere.attn.c), 0.0f, 1.0f);
			ImGui::SliderFloat("d", &(g_wave_param.PointLightSphere.attn.d), 0.0f, 1.0f);
			ImGui::TreePop();
		}

		ImGui::ColorEdit4("ambient_color", (float *)&(g_wave_param.PointLightSphere.ambient_color));
		ImGui::ColorEdit4("diffuse_color", (float *)&(g_wave_param.PointLightSphere.diffuse_color));
		ImGui::SliderFloat("distance",     &(g_wave_param.PointLightSphere.distance), 0.0f, 2.0f);
		ImGui::SliderFloat("fade",         &(g_wave_param.PointLightSphere.fade), 0.0f, 1.0f);

		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("Fog0")){

		ImGui::ColorEdit4("color", (float *)&(g_wave_param.Fog[0].color));

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("density")){
			ImGui::SliderFloat("a", &(g_wave_param.Fog[0].density.a), 0.0f, 1.0f);
			ImGui::SliderFloat("b", &(g_wave_param.Fog[0].density.b), 0.0f, 1.0f);
			ImGui::SliderFloat("c", &(g_wave_param.Fog[0].density.c), 0.0f, 1.0f);
			ImGui::SliderFloat("d", &(g_wave_param.Fog[0].density.d), 0.0f, 20.0f);
			ImGui::TreePop();
		}

		ImGui::ColorEdit4("light_color", (float *)&(g_wave_param.Fog[0].light_color));
		ImGui::SliderFloat("d_scale", &(g_wave_param.Fog[0].d_scale), 0.0f, 4.0f);
		ImGui::SliderFloat("d_offset", &(g_wave_param.Fog[0].d_offset), 0.0f, 5.0f);

		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("Fog1")){

		ImGui::ColorEdit4("color", (float *)&(g_wave_param.Fog[1].color));

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("density")){
			ImGui::SliderFloat("a", &(g_wave_param.Fog[1].density.a), 0.0f, 1.0f);
			ImGui::SliderFloat("b", &(g_wave_param.Fog[1].density.b), 0.0f, 1.0f);
			ImGui::SliderFloat("c", &(g_wave_param.Fog[1].density.c), 0.0f, 1.0f);
			ImGui::SliderFloat("d", &(g_wave_param.Fog[1].density.d), 0.0f, 20.0f);
			ImGui::TreePop();
		}

		ImGui::ColorEdit4("light_color", (float *)&(g_wave_param.Fog[1].light_color));
		ImGui::SliderFloat("d_scale", &(g_wave_param.Fog[1].d_scale), 0.0f, 5.0f);
		ImGui::SliderFloat("d_offset", &(g_wave_param.Fog[1].d_offset), 0.0f, 5.0f);
		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("Sky")){
		ImGui::SliderFloat("theta_phi 0", &(g_wave_param.Sky.theta_phi[0]), -5.0f, 5.0f);
		ImGui::SliderFloat("theta_phi 1", &(g_wave_param.Sky.theta_phi[1]), -5.0f, 5.0f);

		ImGui::ColorEdit4("sun_color",     (float *)&(g_wave_param.Sky.sun_color));
		ImGui::ColorEdit4("zenith_color",  (float *)&(g_wave_param.Sky.zenith_color));
		ImGui::ColorEdit4("horizon_color", (float *)&(g_wave_param.Sky.horizon_color));

		ImGui::SliderFloat("distance",                 &(g_wave_param.Sky.distance), 0.0f, 50.0f);
		ImGui::SliderFloat("sun_power",                &(g_wave_param.Sky.sun_power), 0.0f, 30.0f);
		ImGui::SliderFloat("sun_control",              &(g_wave_param.Sky.sun_control), 0.0f, 1.0f);
		ImGui::SliderFloat("wave_fog_alpha",           &(g_wave_param.Sky.wave_fog_alpha), 0.0f, 1.0f);
		ImGui::SliderFloat("radius",                   &(g_wave_param.Sky.radius), 0.0f, 18.0f);
		ImGui::SliderFloat("xscale",                   &(g_wave_param.Sky.xscale), -3.0f, 3.0f);
		ImGui::SliderFloat("side_angle",               &(g_wave_param.Sky.side_angle), 0.0f, 1.0f);
		ImGui::SliderFloat("horizon_angle",            &(g_wave_param.Sky.horizon_angle), 0.0f, 1.0f);
		ImGui::SliderFloat("horizon_blend_range",      &(g_wave_param.Sky.horizon_blend_range), 0.0f, 1.0f);
		ImGui::SliderFloat("horizon_curvature",        &(g_wave_param.Sky.horizon_curvature), 0.0f, 1.0f);
		ImGui::SliderFloat("sky_blend_start",          &(g_wave_param.Sky.sky_blend_start), 0.0f, 1.0f);
		ImGui::SliderFloat("sky_blend_range",          &(g_wave_param.Sky.sky_blend_range), 0.0f, 1.0f);
		ImGui::SliderFloat("pos_distortion_scale",     &(g_wave_param.Sky.pos_distortion_scale), 0.0f, 1.0f);
		ImGui::SliderFloat("grad_distortion_scale",    &(g_wave_param.Sky.grad_distortion_scale), 0.0f, 1.0f);
		ImGui::SliderFloat("horizon_distortion_scale", &(g_wave_param.Sky.horizon_distortion_scale), 0.0f, 1.0f);

		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("FFTWave")){
		ImGui::SliderFloat("gravity",    &(g_wave_param.FFTWave.gravity),    0.0f, 20.0f);
		ImGui::SliderFloat("A",          &(g_wave_param.FFTWave.A),          -10.0f, 1.0f);
		ImGui::SliderFloat("logA",       &(g_wave_param.FFTWave.logA),       -20.0f, 1.0f); // value range confirmed
		ImGui::SliderFloat("patch_size", &(g_wave_param.FFTWave.patch_size), 0.0f, 200.0f);
		ImGui::SliderFloat("wind_speed", &(g_wave_param.FFTWave.wind_speed), 0.0f, 20.0f);
		ImGui::SliderFloat("wind_dir",   &(g_wave_param.FFTWave.wind_dir),   -6.0f, 6.0f);
		ImGui::SliderFloat("time_step",  &(g_wave_param.FFTWave.time_step),  0.0f, 1.0f);
		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("WaveInstance")){

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("translation")){
			ImGui::SliderFloat("x", &(g_wave_param.WaveInstance.translation.x), -5.0f, 20.0f);
			ImGui::SliderFloat("y", &(g_wave_param.WaveInstance.translation.y), -5.0f, 20.0f);
			ImGui::SliderFloat("z", &(g_wave_param.WaveInstance.translation.z), -5.0f, 20.0f);
			ImGui::SliderFloat("w", &(g_wave_param.WaveInstance.translation.w), -5.0f, 20.0f);
			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("rotation")){
			ImGui::SliderFloat("x", &(g_wave_param.WaveInstance.rotation.x), -3.0f, 3.0f);
			ImGui::SliderFloat("y", &(g_wave_param.WaveInstance.rotation.y), -3.0f, 3.0f);
			ImGui::SliderFloat("z", &(g_wave_param.WaveInstance.rotation.z), -3.0f, 3.0f);
			ImGui::SliderFloat("w", &(g_wave_param.WaveInstance.rotation.w), -3.0f, 3.0f);
			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("scale")){
			ImGui::SliderFloat("x", &(g_wave_param.WaveInstance.scale.x), 0.0f, 20.0f);
			ImGui::SliderFloat("y", &(g_wave_param.WaveInstance.scale.y), 0.0f, 20.0f);
			ImGui::SliderFloat("z", &(g_wave_param.WaveInstance.scale.z), 0.0f, 20.0f);
			ImGui::SliderFloat("w", &(g_wave_param.WaveInstance.scale.w), 0.0f, 20.0f);
			ImGui::TreePop();
		}

		if(g_wave_param.color_index == 0x1F || 1){
			ImGui::ColorEdit4("center_color", (float *)&(g_wave_param.WaveInstance.center_color));
			ImGui::ColorEdit4("edge_color",   (float *)&(g_wave_param.WaveInstance.edge_color));
		}else{
			ImGui::ColorEdit4("center_color", (float *)&(g_wave_param.WaveInstance.edge_color));
			ImGui::ColorEdit4("edge_color",   (float *)&(g_wave_param.WaveInstance.center_color));
		}

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("cross_section")){
			ImGui::SliderFloat("x", &(g_wave_param.WaveInstance.cross_section.x), -5.0f, 20.0f);
			ImGui::SliderFloat("y", &(g_wave_param.WaveInstance.cross_section.y), -5.0f, 20.0f);
			ImGui::SliderFloat("z", &(g_wave_param.WaveInstance.cross_section.z), -5.0f, 20.0f);
			ImGui::SliderFloat("w", &(g_wave_param.WaveInstance.cross_section.w), -5.0f, 20.0f);
			ImGui::TreePop();
		}

		ImGui::SetNextItemOpen(true, ImGuiCond_Once);
		if(ImGui::TreeNode("cross_section_flat")){
			ImGui::SliderFloat("x", &(g_wave_param.WaveInstance.cross_section_flat.x), -5.0f, 20.0f);
			ImGui::SliderFloat("y", &(g_wave_param.WaveInstance.cross_section_flat.y), -5.0f, 20.0f);
			ImGui::SliderFloat("z", &(g_wave_param.WaveInstance.cross_section_flat.z), -5.0f, 20.0f);
			ImGui::SliderFloat("w", &(g_wave_param.WaveInstance.cross_section_flat.w), -5.0f, 20.0f);
			ImGui::TreePop();
		}

		ImGui::SliderFloat("distortion_scale", &(g_wave_param.WaveInstance.distortion_scale), 0.0f, 1.0f);   // value range confirmed
		ImGui::SliderFloat("global_scale",     &(g_wave_param.WaveInstance.global_scale),     0.0f, 10.0f);
		ImGui::SliderFloat("uv_scale",         &(g_wave_param.WaveInstance.uv_scale),         0.0f, 1.0f);   // value range confirmed
		ImGui::SliderFloat("uv_rotate",        &(g_wave_param.WaveInstance.uv_rotate),        0.0f, 359.0f); // value range confirmed
		ImGui::SliderFloat("center_power",     &(g_wave_param.WaveInstance.center_power),     0.0f, 20.0f);
		ImGui::SliderFloat("edge_power",       &(g_wave_param.WaveInstance.edge_power),       0.0f, 20.0f);
		ImGui::SliderFloat("decay",            &(g_wave_param.WaveInstance.decay),            0.0f, 20.0f);
		ImGui::SliderFloat("shadow_nor_blend", &(g_wave_param.WaveInstance.shadow_nor_blend), 0.0f, 20.0f);

		ImGui::TreePop();
	}

	ImGui::SetNextItemOpen(false, ImGuiCond_Once);
	if(ImGui::TreeNode("WaveRenderer")){
		ImGui::SliderFloat("edge_fog 0",    &(g_wave_param.WaveRenderer.edge_fog[0]),   0.0f, 1.0f); // value range confirmed
		ImGui::SliderFloat("edge_fog 1",    &(g_wave_param.WaveRenderer.edge_fog[1]),   0.0f, 1.0f); // value range confirmed
		ImGui::SliderFloat("pitch_max",     &(g_wave_param.WaveRenderer.pitch_max),     -3.0f, 5.0f);
		ImGui::SliderFloat("pitch_rest",    &(g_wave_param.WaveRenderer.pitch_rest),    -3.0f, 5.0f);
		ImGui::SliderFloat("pitch_min",     &(g_wave_param.WaveRenderer.pitch_min),     -3.0f, 5.0f);
		ImGui::SliderFloat("pitch_debug",   &(g_wave_param.WaveRenderer.pitch_debug),   -3.0f, 5.0f);
		ImGui::SliderFloat("roll_max",      &(g_wave_param.WaveRenderer.roll_max),      0.0f, 5.0f);
		ImGui::SliderFloat("roll_max_zone", &(g_wave_param.WaveRenderer.roll_max_zone), 0.0f, 5.0f);
		ImGui::SliderFloat("zoom",          &(g_wave_param.WaveRenderer.zoom),          0.0f, 5.0f);
		ImGui::SliderFloat("camera_chase",  &(g_wave_param.WaveRenderer.camera_chase),  0.0f, 5.0f);
		ImGui::TreePop();
	}

	return 0;
}

int psp2wpp_settings_draw(void){

	static bool s_open_main_settings = true;
	static bool s_open_tool_settings = true;

	{
		ImGui::Begin("Settings");

		if(ImGui::Button("Main Seetings")){
			s_open_main_settings = true;
		}

		if(ImGui::Button("Tool Seetings")){
			s_open_tool_settings = true;
		}

		ImGui::End();
	}

	if(s_open_main_settings == true){
		ImGui::Begin("Main Settings", &s_open_main_settings);
		pthread_mutex_lock(&g_mutex);
		psp2wpp_settings_draw_main_settings();
		pthread_mutex_unlock(&g_mutex);
		ImGui::End();
	}

	if(s_open_tool_settings == true){
		ImGui::Begin("Tool Settings", &s_open_tool_settings);

		ImGui::Checkbox("Auto Update", &g_auto_update);

		// ImGui::SameLine();
		if(ImGui::Button("Update")){
			g_exec_update = true;
		}

		// ImGui::SameLine();
		if(ImGui::Button("Load Current Wave")){
			g_exec_load = true;
		}

		if(ImGui::Button("Save waveparam to PC")){
			s_save_dialog_to_pc = true;
		}

		ImGui::SameLine();
		if(ImGui::Button("Save waveparam to PS Vita")){
			s_save_dialog_to_vita = true;
		}

		if(ImGui::IsItemHovered()){
			ImGui::SetTooltip("Save waveparam to \"ux0:/data/waveparam.bin\".");
		}

		ImGui::End();
	}

	if(s_save_dialog_to_pc == true){
		ImGui::Begin("Save waveparam to PC");

		ImGui::InputText("file name", save_name, sizeof(save_name));

		if(ImGui::Button("Save")){

			char save_path[0x400];

			if(save_name[0] == 0){
				printf("save file name length is 0. failed.\n");
			}else{
				mkdir("./waveparam");

				snprintf(save_path, sizeof(save_path), "./waveparam/%s", save_name);

				pthread_mutex_lock(&g_mutex);

				FILE *fp = fopen(save_path, "wb");
				if(fp != NULL){
					fwrite(&g_wave_param, sizeof(g_wave_param), 1, fp);
					fclose(fp);
				}

				pthread_mutex_unlock(&g_mutex);

				printf("waveparam is saved!\n");
			}

			s_save_dialog_to_pc = false;
		}

		ImGui::SameLine();
		if(ImGui::Button("Cancel")){
			s_save_dialog_to_pc = false;
		}

		ImGui::End();
	}

	if(s_save_dialog_to_vita == true){
		ImGui::Begin("Save waveparam to PS Vita");

		ImGui::Text("Warning\nSaving waveparam to PS Vita will overwrite previously saved waveparam.\nDo you still want to save?");

		if(ImGui::Button("Save")){
			s_exec_save_to_vita = true;
		}

		ImGui::SameLine();
		if(ImGui::Button("Cancel")){
			s_save_dialog_to_vita = false;
		}

		ImGui::End();
	}

	return 0;
}

// Main code
int main(int, char**){

	printf("#\n");
	printf("# psp2wpp remote configuration tool\n");
	printf("#   build date: " __DATE__ " " __TIME__ " +0900\n");
	printf("#\n");

	g_wave_param.magic = SCE_WAVE_PARAM_MAGIC;
	g_wave_param.version = 1;

	FILE *fp = fopen("waveparam.bin", "rb");
	if(fp != NULL){
		fread(&g_wave_param, sizeof(g_wave_param), 1, fp);
		fclose(fp);

		printf("Preloading waveparam.bin\n");
	}

	pthread_t thread;

	pthread_mutex_init(&g_mutex, NULL);
	pthread_create(&thread, NULL, usb_comm_thread, NULL);

	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGui Example", nullptr };
	::RegisterClassExW(&wc);
	HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"psp2wpp remote configuration", WS_OVERLAPPEDWINDOW, 100, 100, 673, 600, nullptr, nullptr, wc.hInstance, nullptr);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClassW(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	// io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX10_Init(g_pd3dDevice);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != nullptr);

	io.Fonts->AddFontFromFileTTF("./font.ttf", 15.0f);


	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	// Main loop
	bool done = false;
	while (!done)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// See the WndProc() function below for our to dispatch events to the Win32 backend.
		MSG msg;
		while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done)
			break;

		// Handle window resize (we don't resize directly in the WM_SIZE handler)
		if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
		{
			CleanupRenderTarget();
			g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
			g_ResizeWidth = g_ResizeHeight = 0;
			CreateRenderTarget();
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX10_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		if (0 && show_demo_window)
			ImGui::ShowDemoWindow(&show_demo_window);

		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
		if(0){
			static float f = 0.0f;
			static int counter = 0;

			ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

			ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
			ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
			ImGui::Checkbox("Another Window", &show_another_window);

			ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
			ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

			if(ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
				counter++;

			ImGui::SameLine();
			ImGui::Text("counter = %d", counter);
			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);

			ImGui::End();
		}

		{
			psp2wpp_settings_draw();
		}

		// 3. Show another simple window.
		if (show_another_window)
		{
			ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
			ImGui::Text("Hello from another window!");
			if (ImGui::Button("Close Me"))
				show_another_window = false;
			ImGui::End();
		}

		// Rendering
		ImGui::Render();
		const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
		g_pd3dDevice->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
		g_pd3dDevice->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
		ImGui_ImplDX10_RenderDrawData(ImGui::GetDrawData());

		g_pSwapChain->Present(1, 0); // Present with vsync
		//g_pSwapChain->Present(0, 0); // Present without vsync
	}

	ImGui_ImplDX10_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClassW(wc.lpszClassName, wc.hInstance);

	pthread_mutex_lock(&g_mutex);
	g_alive = 0;
	pthread_mutex_unlock(&g_mutex);
	pthread_join(thread, NULL);
	pthread_mutex_destroy(&g_mutex);

	printf("bye!\n");

	return 0;
}
