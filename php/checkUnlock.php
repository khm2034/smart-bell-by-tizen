<?php 
    $con = mysqli_connect("localhost","root","sqlrootpw","db");
    if(!$con)
    {
        echo "MySQL 접속 에러 : ";
        echo mysqli_connect_error();
        exit();
    }
    mysqli_query("set names utf8");

    $data = file_get_contents('php://input');
    $arr = json_decode($data, TRUE);
    $key = $arr['UNLOCK'];
    $key = "'". $key. "'";
    $result = mysqli_query($con, "SELECT * FROM manager WHERE m_key = $key");
    $response = array();
    if( $row = mysqli_fetch_array($result))
    {
         $response["success"] = "1";
         echo json_encode($response);    
    }
    else
    {  
        $response["success"] = "0";
        echo json_encode($response);
    }
?>