#include "client.h"

#include "visual_nurb.h"
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include <opennurbs_public.h>

namespace VN
{
    namespace
    {
        double delta_time()
        {
            static bool counting_started = false;
            static long long last_tick = 0;

            auto now = std::chrono::high_resolution_clock::now();
            long long cur_tick = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
            double ts((cur_tick - last_tick) / 1000.0);
            last_tick = cur_tick;
            if (!counting_started)
            {
                counting_started = true;
                return 0.0;
            }

            return ts;
        }
    }

    vn_client_instance::~vn_client_instance()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        delete m_srf;

        gluDeleteNurbsRenderer(m_nurbs);
        delete m_window_user_data.m_cam;
        glfwTerminate();

    }

    void vn_client_instance::init()
    {
        init_plot_window_and_camera();
        init_nurb_renderer();
        init_gui();
    }

    void vn_client_instance::exec()
    {
        glClearColor(0.2, 0.2, 0.2, 0.8);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(m_window))
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            /* Update camera matrix */
            m_window_user_data.m_cam->on_update(delta_time());

            /* ------------- render jobs begin -------------------- */
            render_begin();

            render_nurbs();
            //render_gui();

            render_end();
            /* ------------- render jobs end ---------------------- */

            /* Swap front and back buffers */
            glfwSwapBuffers(m_window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    int vn_client_instance::init_plot_window_and_camera()
    {
        /* Initialize the library */
        if (!glfwInit())
            return -1;

        /* Create a windowed mode window and its OpenGL context */
        m_window = glfwCreateWindow(640, 480, "VisualNurb", NULL, NULL);
        if (!m_window)
        {
            glfwTerminate();
            return -1;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            return -1;
        }

        /* Initial camera controller */
        m_window_user_data.m_cam = new camera(m_window, 45.f, 1.5f, 0.1f, 1000.f);

        glfwSetWindowUserPointer(m_window, &m_window_user_data);

        // add event callbacks
        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            ::VN::mouse_scrolled_event event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            ((window_user_data*)glfwGetWindowUserPointer(window))->m_cam->on_event(event);
        });

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
            ((window_user_data*)glfwGetWindowUserPointer(window))->m_width = width;
            ((window_user_data*)glfwGetWindowUserPointer(window))->m_height = height;
        });

        return 0;
    }

    int vn_client_instance::init_nurb_renderer()
    {
        frame_buffer_specification spec;
        spec.width = 1000;
        spec.height = 800;
        spec.attachments = {
            frame_buffer_texture_format::kRGBA8,
            //frame_buffer_texture_format::kRedInteger,
            //frame_buffer_texture_format::kDepth
        };

        m_frame_buffer = std::make_unique<frame_buffer>(spec);

        m_nurbs = gluNewNurbsRenderer();
        gluNurbsProperty(m_nurbs, GLU_SAMPLING_TOLERANCE, 25.0);
        gluNurbsProperty(m_nurbs, GLU_DISPLAY_MODE, GLU_FILL);

        return 0;
    }

    int vn_client_instance::init_gui()
    {
        IMGUI_CHECKVERSION();

        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        io.FontGlobalScale = 2.0f;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

        ImGui_ImplGlfw_InitForOpenGL(m_window, true);
        ImGui_ImplOpenGL3_Init("#version 330");

        return 0;
    }

    void vn_client_instance::render_begin()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        static bool dock_enabled = true;

        static ImGuiDockNodeFlags dock_flags = ImGuiDockNodeFlags_None;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

        if (true) {
            ImGuiViewport* gui_viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(gui_viewport->Pos);
            ImGui::SetNextWindowSize(gui_viewport->Size);
            ImGui::SetNextWindowViewport(gui_viewport->ID);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (dock_flags & ImGuiDockNodeFlags_PassthruCentralNode) {
            window_flags |= ImGuiWindowFlags_NoBackground;
        }

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("main dock space", &dock_enabled, window_flags);
        ImGui::PopStyleVar();

        if (true) {
            ImGui::PopStyleVar();
        }

        ImGuiIO& io = ImGui::GetIO();
        ImGuiStyle& style = ImGui::GetStyle();
        float min_window_size = style.WindowMinSize.x;
        style.WindowMinSize.x = 400.f;
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable) {
            ImGuiID dock_id = ImGui::GetID("main dock space");
            ImGui::DockSpace(dock_id, ImVec2(0.f, 0.f), dock_flags);
        }

        style.WindowMinSize.x = min_window_size;
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
    }

    void vn_client_instance::render_end()
    {
        ImGui::PopStyleVar();
        ImGui::End();

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2((float)m_window_user_data.m_width, (float)m_window_user_data.m_height);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void vn_client_instance::render_gui()
    {
        ImGui::Begin("Plot preferences");

        if (is_connected())
        {
            if (ImGui::Button("Disconnect"))
            {
                vn_log("CONNECTION", "Ready to disconnect.");
                m_connected_to = false;
            }
        }
        else
        {
            if (ImGui::Button("Connect"))
            {
                vn_log("CONNECTION", "Ready to connect.");
                m_connected_to = true;
            }
        }

        ImGui::End();
    }

    void vn_client_instance::render_nurbs()
    {
        ImGui::Begin("Plot Nurbs");

        ImVec2 viewport_min_region = ImGui::GetWindowContentRegionMin();
        ImVec2 viewport_max_region = ImGui::GetWindowContentRegionMax();
        ImVec2 viewport_offset = ImGui::GetWindowPos();

        double width = viewport_max_region.x - viewport_min_region.x;
        double height = viewport_max_region.y - viewport_min_region.y;
        if (width > 0 && height > 0)
        {
            m_frame_buffer->on_resize(width, height);
        }
        else
        {
            ImGui::End();
            return;
        }

        m_frame_buffer->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        m_frame_buffer->clear_attachment(1, 0);

        {
            std::lock_guard<std::mutex> lkm(m_mutex);

            if (m_srf)
                draw_nurbs_surf();

            //if (m_crv)
            draw_nurbs_curve(m_crv);
        }

        m_frame_buffer->unbind();

        ImGui::Image(reinterpret_cast<void*>(m_frame_buffer->color_attachment_renderer_id(0))
            , ImVec2(m_frame_buffer->specification().width, m_frame_buffer->specification().height));

        ImGui::End();
    }

    int vn_client_instance::draw_nurbs_surf()
    {
        auto m_cam = m_window_user_data.m_cam;

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(m_cam->m_vertical_fov, m_cam->m_aspect_ratio, m_cam->m_near_clip, m_cam->m_far_clip);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(m_cam->eye().x(), m_cam->eye().y(), m_cam->eye().z(), m_cam->look_at().x(), m_cam->look_at().y(), m_cam->look_at().z(),
            m_cam->up_direction().x(), m_cam->up_direction().y(), m_cam->up_direction().z());

        std::vector<float> ctrl_points;
        int u_ctrl_num = 0;
        int v_ctrl_num = 0;
        std::vector<float> u_knots;
        int u_knot_num = 0;
        std::vector<float> v_knots;
        int v_knot_num = 0;
        int u_order = 0;
        int v_order = 0;
        int dim = 3;


        u_knot_num = m_srf->u.num_kt;
        for (int i = 0; i < m_srf->u.num_kt; i++)
            u_knots.emplace_back(m_srf->u.knots[i]);

        v_knot_num = m_srf->v.num_kt;
        for (int i = 0; i < m_srf->v.num_kt; i++)
            v_knots.emplace_back(m_srf->v.knots[i]);

        u_order = m_srf->u.degree + 1;
        v_order = m_srf->v.degree + 1;
        u_ctrl_num = u_knot_num - u_order;
        v_ctrl_num = v_knot_num - v_order;

        switch (m_srf->cp.dim)
        {
        case 2:
        {

            for (int i = 0; i < m_srf->cp.num_cp; i++)
            {
                for (int j = 0; j < 2; j++)
                    ctrl_points.emplace_back(m_srf->cp.list[i * 2 + j]);
                ctrl_points.emplace_back(0.0);
            }
            break;
        }
        case 3:
        {
            for (int i = 0; i < m_srf->cp.num_cp; i++)
            {
                for (int j = 0; j < 3; j++)
                    ctrl_points.emplace_back(m_srf->cp.list[i * 3 + j]);
            }
            break;
        }
        case 4:
        {
            for (int i = 0; i < m_srf->cp.num_cp; i++)
            {
                for (int j = 0; j < 3; j++)
                    ctrl_points.emplace_back(m_srf->cp.list[i * 4 + j] * m_srf->cp.list[i * 4 + 3]);
            }
            break;
        }
        }

        int u_stride = v_ctrl_num * dim;
        int v_stride = dim;

        glPushMatrix();
        //绘制控制点与控制线
        glScaled(0.2, 0.2, 0.2);

        glPointSize(4.0f);
        glColor3f(0.0, 0.0, 1.0);
        glColor3f(0, 0, 1);
        glBegin(GL_POINTS);
        for (int i = 0; i < u_ctrl_num; i++)
        {
            for (int j = 0; j < v_ctrl_num; j++)
                glVertex3fv(&ctrl_points[i * v_ctrl_num * dim + j * dim]);
        }
        glEnd();
        //绘制控制线
        glLineWidth(1.5f);
        glColor3f(0.0, 1.0, 1.0);
        for (int i = 0; i < u_ctrl_num; i++)
        {
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < v_ctrl_num; j++)
                glVertex3fv(&ctrl_points[i * v_ctrl_num * dim + j * dim]);
            glEnd();
        }
        for (int i = 0; i < v_ctrl_num; i++)
        {
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < u_ctrl_num; j++)
                glVertex3fv(&ctrl_points[j * v_ctrl_num * dim + i * dim]);
            glEnd();
        }

        gluNurbsProperty(m_nurbs, GLU_SAMPLING_TOLERANCE, 25); //设置属性
        gluNurbsProperty(m_nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
        gluBeginSurface(m_nurbs);//开始绘制
        gluNurbsSurface(m_nurbs,
            u_knot_num, u_knots.data(),
            v_knot_num, v_knots.data(),
            u_stride,
            v_stride,
            ctrl_points.data(),
            u_order, v_order,
            GL_MAP2_VERTEX_3);

        gluEndSurface(m_nurbs); //结束绘制

        for (int j = 0; j <= 8; j++)
        {
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i <= 30; i++)
                glEvalCoord2f((GLfloat)i / 30.0, (GLfloat)j / 8.0);
            glEnd();
            glBegin(GL_LINE_STRIP);
            for (int i = 0; i <= 30; i++)
                glEvalCoord2f((GLfloat)j / 8.0, (GLfloat)i / 30.0);
            glEnd();
        }
        glPopMatrix();

        for (int i = 0; i < m_srf->num_loop; i++)
        {
            for (int j = 0; j < m_srf->list_loop[i].num_cv; j++)
            {
                draw_nurbs_curve(&m_srf->list_loop[i].list_cv[j]);
            }
        }

        return 0;
    }

    void check_open_gl_error(const char* function) {
        GLenum error = glGetError();
        while (error != GL_NO_ERROR) {
            std::cerr << "OpenGL Error in " << function << ": " << error << std::endl;
            error = glGetError();
        }
    }

    int vn_client_instance::draw_nurbs_curve(VsNurbCurv* crv)
    {
        auto m_cam = m_window_user_data.m_cam;

        //glMatrixMode(GL_PROJECTION);
        //glLoadIdentity();
        //gluPerspective(m_cam->m_vertical_fov, m_cam->m_aspect_ratio, m_cam->m_near_clip, m_cam->m_far_clip);

        //glMatrixMode(GL_MODELVIEW);
        //glLoadIdentity();
        //gluLookAt(m_cam->eye().x(), m_cam->eye().y(), m_cam->eye().z(), m_cam->look_at().x(), m_cam->look_at().y(), m_cam->look_at().z(),
        //    m_cam->up_direction().x(), m_cam->up_direction().y(), m_cam->up_direction().z());

        GLUquadric* quadric = gluNewQuadric(); // 创建一个新的二次曲面对象
        gluSphere(quadric, 0.5, 50, 50); // 绘制球体，半径为1.0，50个纬线和50个经线

        gluDeleteQuadric(quadric); // 删除二次曲面对象

        check_open_gl_error("draw_nurbs_curve");

        return 0;
    }

    void vn_client_instance::handle_message(const char* buffer, int nbytes)
    {
        seralize_stream ss(buffer, nbytes);

        int type = -1;
        ss.read(type);

        std::lock_guard<std::mutex> lkm(m_mutex);
        switch (type)
        {
        case VN_FLAG_CURVE:
            {
                vn_log("MESSAGE", "Nurbs curve plot received.");
                delete m_crv;
                m_crv = new VsNurbCurv;
                ss.read(*m_crv);

                ::VN::yaml_serializer::dump("test.yaml", *m_crv);
                ::VN::VsNurbCurv crv2;
                ::VN::yaml_serializer::load("test.yaml", crv2);

                break;
            }
        case VN_FLAG_SURFACE:
            {
                vn_log("MESSAGE", "Nurbs surface plot received.");
                delete m_srf;
                m_srf = new VsNurbSurf;
                ss.read(*m_srf);
                break;
            }
        default:
            {
                // shouldn't, check it
                __debugbreak();
            }
        }
    }

}